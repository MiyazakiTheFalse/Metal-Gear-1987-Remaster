//! Player-driven pacing guidance for the Gray Fox prison sequence.
//! No stage has a hard timer; elapsed time affects optional hint delivery only.
//! This is an adaptation of the MSX2 rescue, not J2ME-verified original timing.
use crate::gray_fox::{
    CAPTURED, FIRST_WALL_BREACHED, GRAY_FOX_RESCUED,
    EXIT_WALL_BREACHED, LEFT_PRISON, ALL_FLAGS,
};

#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct PacingOutput {
    pub phase: u32,
    pub target_intensity: f32,
    pub hint_level: u32,
    pub should_suppress_new_pressure: u32,
}

pub const CAPTURE_BEAT: u32 = 0;
pub const CELL_DISCOVERY: u32 = 1;
pub const BREAKTHROUGH: u32 = 2;
pub const REVELATION: u32 = 3;
pub const ESCAPE_PRESSURE: u32 = 4;
pub const RECOVERY: u32 = 5;

/// 'Under alert' is an AI fact, not an editor pacing request. Never cancel
/// legitimate detection just because a narrative beat wants a quiet moment.
pub fn evaluate(flags: u32, seconds_without_progress: f32, under_alert: bool)
    -> Option<PacingOutput>
{
    if flags & !ALL_FLAGS != 0 { return None; }
    let (phase, intensity, soft_pressure) = if flags & CAPTURED == 0 {
        (CAPTURE_BEAT, 0.35, false)
    } else if flags & FIRST_WALL_BREACHED == 0 {
        (CELL_DISCOVERY, 0.15, true)
    } else if flags & GRAY_FOX_RESCUED == 0 {
        (BREAKTHROUGH, 0.40, false)
    } else if flags & EXIT_WALL_BREACHED == 0 {
        (REVELATION, 0.20, true)
    } else if flags & LEFT_PRISON == 0 {
        (ESCAPE_PRESSURE, 0.82, false)
    } else {
        (RECOVERY, 0.05, true)
    };
    let since = if seconds_without_progress.is_finite() {
        seconds_without_progress.max(0.0)
    } else { 0.0 };
    let hint = if under_alert || phase == CAPTURE_BEAT || phase == RECOVERY {
        0
    } else if since >= 35.0 {
        2
    } else if since >= 14.0 {
        1
    } else {
        0
    };
    Some(PacingOutput {
        phase, target_intensity: intensity,
        hint_level: hint,
        should_suppress_new_pressure: (soft_pressure && !under_alert) as u32,
    })
}

/// # Safety
/// The caller must pass a correctly aligned, writable PacingOutput pointer.
#[no_mangle]
pub unsafe extern "C" fn mg_gray_fox_pacing(
    flags: u32, elapsed: f32, under_alert: u32, out: *mut PacingOutput
) -> u32 {
    if out.is_null() { return 0; }
    match evaluate(flags, elapsed, under_alert != 0) {
        Some(pacing) => {
            unsafe { std::ptr::write(out, pacing); }
            1
        }
        None => 0,
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::gray_fox::{EQUIPMENT_CONFISCATED, EXIT_WALL_BREACHED};

    #[test] fn tension_has_revelation_rest_and_escape_peak() {
        let initial = CAPTURED | EQUIPMENT_CONFISCATED;
        let a = evaluate(initial, 0.0, false).unwrap();
        let b = evaluate(initial | FIRST_WALL_BREACHED, 0.0, false).unwrap();
        let c = evaluate(initial | FIRST_WALL_BREACHED | GRAY_FOX_RESCUED, 0.0, false).unwrap();
        let d = evaluate(initial | FIRST_WALL_BREACHED | GRAY_FOX_RESCUED
            | EXIT_WALL_BREACHED, 0.0, false).unwrap();
        assert_eq!((a.phase, b.phase, c.phase, d.phase),
                   (CELL_DISCOVERY, BREAKTHROUGH, REVELATION, ESCAPE_PRESSURE));
        assert!(b.target_intensity > a.target_intensity);
        assert!(c.target_intensity < b.target_intensity);
        assert!(d.target_intensity > b.target_intensity);
    }

    #[test] fn hints_are_optional_and_avoid_combat_overlay() {
        let state = CAPTURED | EQUIPMENT_CONFISCATED;
        assert_eq!(evaluate(state, 13.9, false).unwrap().hint_level, 0);
        assert_eq!(evaluate(state, 14.0, false).unwrap().hint_level, 1);
        assert_eq!(evaluate(state, 35.0, false).unwrap().hint_level, 2);
        assert_eq!(evaluate(state, 100.0, true).unwrap().hint_level, 0);
    }

    #[test] fn there_is_no_blocking_wait_in_pacing_model() {
        let state = CAPTURED | EQUIPMENT_CONFISCATED;
        assert!(evaluate(state, 0.0, false).is_some());
        // The actual action transition remains legal at 0 seconds.
        assert!(crate::gray_fox::transition(
            state, crate::gray_fox::ACTION_BREACH_FIRST_WALL).is_some());
    }

    #[test] fn invalid_state_and_pointer_are_rejected() {
        assert!(evaluate(1<<20, 0.0, false).is_none());
        assert_eq!(unsafe {
            mg_gray_fox_pacing(0, 0.0, 0, std::ptr::null_mut())
        }, 0);
    }
}
