//! Engine-neutral stealth simulation for the Metal Gear remaster.
//! Java ME bytecode is a reference, NOT something this crate pretends to execute.
pub mod resource;
pub mod mission_events;

#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct Vec2 { pub x: f32, pub y: f32 }

impl Vec2 {
    fn distance(self, other: Self) -> f32 {
        (self.x - other.x).hypot(self.y - other.y)
    }
    fn toward(self, other: Self) -> Self {
        let dx = other.x - self.x;
        let dy = other.y - self.y;
        let magnitude = dx.hypot(dy);
        if magnitude < 0.00001 { Self { x: 0.0, y: 0.0 } }
        else { Self { x: dx / magnitude, y: dy / magnitude } }
    }
    fn normalized(self) -> Self { Vec2 { x: 0.0, y: 0.0 }.toward(self) }
}

pub const PATROL: u32 = 0;
pub const INVESTIGATE: u32 = 1;
pub const SEARCH: u32 = 2;
pub const ALERT: u32 = 3;

/// Inputs are horizontal world coordinates in metres. LoS comes from Unreal physics.
#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct GuardFrame {
    pub guard_pos: Vec2,
    pub guard_forward: Vec2,
    pub player_pos: Vec2,
    pub vision_range_m: f32,
    pub vision_half_angle_deg: f32,
    pub hearing_range_m: f32,
    pub visibility: f32,
    pub noise: f32,
    pub suspicion: f32,
    pub delta_seconds: f32,
    pub line_of_sight: u32,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct GuardResult {
    pub suspicion: f32,
    pub state: u32,
    pub sees_player: u32,
    pub hears_player: u32,
    pub visual_threat: f32,
    pub sound_threat: f32,
}

fn unit_clamped(value: f32) -> f32 {
    if value.is_finite() { value.clamp(0.0, 1.0) } else { 0.0 }
}

/// Deterministic awareness step; no Unreal dependency, allocations or timers.
pub fn step_guard(frame: GuardFrame) -> GuardResult {
    let dt = if frame.delta_seconds.is_finite() {
        frame.delta_seconds.clamp(0.0, 0.1)
    } else { 0.0 };
    let distance = frame.guard_pos.distance(frame.player_pos);
    let facing = frame.guard_forward.normalized();
    let toward = frame.guard_pos.toward(frame.player_pos);
    let dot = (facing.x * toward.x + facing.y * toward.y).clamp(-1.0, 1.0);
    let half_angle = if frame.vision_half_angle_deg.is_finite() {
        frame.vision_half_angle_deg.clamp(1.0, 179.0)
    } else { 45.0 };
    let has_sight = frame.line_of_sight != 0
        && frame.vision_range_m.is_finite() && frame.vision_range_m > 0.0
        && distance <= frame.vision_range_m
        && dot >= half_angle.to_radians().cos();
    let visual = if has_sight {
        unit_clamped(frame.visibility) * (1.0 - distance / frame.vision_range_m).max(0.0)
    } else { 0.0 };
    let sound = if frame.hearing_range_m.is_finite() && frame.hearing_range_m > 0.0
        && distance <= frame.hearing_range_m {
        unit_clamped(frame.noise) * (1.0 - distance / frame.hearing_range_m).max(0.0)
    } else { 0.0 };
    let gain = dt * (visual * 2.4 + sound * 1.1);
    let decay = if gain > 0.005 { 0.0 } else { dt * 0.12 };
    let suspicion = (unit_clamped(frame.suspicion) + gain - decay).clamp(0.0, 1.0);
    let state = if suspicion >= 0.99 { ALERT }
        else if suspicion >= 0.68 { SEARCH }
        else if suspicion >= 0.22 { INVESTIGATE }
        else { PATROL };
    GuardResult {
        suspicion, state, sees_player: has_sight as u32,
        hears_player: (sound > 0.01) as u32,
        visual_threat: visual, sound_threat: sound
    }
}

/// Versioned native ABI: both sides must compile against the same definitions.
#[no_mangle]
pub extern "C" fn mg_core_abi_version() -> u32 { 1 }

/// # Safety
/// Callers must pass valid, correctly aligned pointers for both structs.
#[no_mangle]
pub unsafe extern "C" fn mg_step_guard(frame: *const GuardFrame, output: *mut GuardResult) -> u32 {
    if frame.is_null() || output.is_null() { return 0; }
    let snapshot = unsafe { std::ptr::read(frame) };
    unsafe { std::ptr::write(output, step_guard(snapshot)) };
    1
}

#[cfg(test)]
mod tests {
    use super::*;
    fn sample() -> GuardFrame {
        GuardFrame {
            guard_pos: Vec2{x:0.0,y:0.0}, guard_forward: Vec2{x:1.0,y:0.0},
            player_pos: Vec2{x:1.0,y:0.0}, vision_range_m: 10.0,
            vision_half_angle_deg: 40.0, hearing_range_m: 10.0,
            visibility: 1.0, noise: 0.0, suspicion: 0.0,
            delta_seconds: 0.1, line_of_sight: 1
        }
    }
    #[test] fn sees_target_ahead() {
        assert_eq!(step_guard(sample()).sees_player, 1);
    }
    #[test] fn ignores_target_behind() {
        let mut f=sample(); f.player_pos.x=-2.0;
        assert_eq!(step_guard(f).sees_player, 0);
    }
    #[test] fn cover_blocks_sight() {
        let mut f=sample(); f.line_of_sight=0;
        assert_eq!(step_guard(f).sees_player, 0);
    }
    #[test] fn noise_causes_investigation() {
        let mut f=sample(); f.line_of_sight=0; f.noise=1.0; f.suspicion=0.21;
        assert_eq!(step_guard(f).state, INVESTIGATE);
    }
    #[test] fn awareness_decays_in_silence() {
        let mut f=sample(); f.line_of_sight=0; f.suspicion=0.8;
        assert!(step_guard(f).suspicion < 0.8);
    }
    #[test] fn invalid_numbers_do_not_poison_output() {
        let mut f=sample(); f.noise=f32::NAN; f.suspicion=f32::NAN;
        assert!(step_guard(f).suspicion.is_finite());
    }
    #[test] fn ffi_rejects_null() {
        assert_eq!(unsafe {mg_step_guard(std::ptr::null(), std::ptr::null_mut())},0);
    }
}
