//! Player-driven pacing guidance for the Gray Fox prison sequence.
//! No stage has a hard timer; elapsed time affects optional hint delivery only.
//! This is an adaptation of the MSX2 rescue, not J2ME-verified original timing.
use crate::gray_fox::{
    CAPTURED, FIRST_WALL_BREACHED, GRAY_FOX_RESCUED,
    EXIT_WALL_BREACHED, LEFT_PRISON, SHOTMAKER_ENCOUNTERED,
    GEAR_RECOVERED, SHOTMAKER_DEFEATED, CARD3_RECOVERED,
    DETENTION_INTELLIGENCE, SLICE_COMPLETED, ALL_FLAGS,
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
pub const SHOTMAKER_THREAT: u32 = 6;
pub const SHOTMAKER_RECOVERY_PUZZLE: u32 = 7;
pub const SHOTMAKER_FIGHT: u32 = 8;
pub const DETENTION_APPROACH: u32 = 9;
pub const AFTERMATH_SEARCH: u32 = 10;
pub const CHAPTER_EXIT: u32 = 11;

// Provisional tuning candidates, NOT empirically optimal delays. The engine
// further suppresses hints while the player is meaningfully investigating.
pub const SOFT_HINT_AFTER_SECONDS: f32 = 28.0;
pub const STRONG_HINT_AFTER_SECONDS: f32 = 75.0;

/// 'Under alert' is an AI fact, not an editor pacing request. Never cancel
/// legitimate detection just because a narrative beat wants a quiet moment.
pub fn evaluate(flags: u32, seconds_without_progress: f32, under_alert: bool)
    -> Option<PacingOutput>
{
    if flags & !ALL_FLAGS != 0 { return None; }
    // Each phase is a change in the player's *information and capability*.
    // Higher intensity does NOT spawn enemies or falsely manufacture alerts.
    let (phase, intensity, soft_pressure) = if flags & CAPTURED == 0 {
        if flags & DETENTION_INTELLIGENCE != 0 {
            (DETENTION_APPROACH, 0.52, false)
        } else {
            (CAPTURE_BEAT, 0.22, true)
        }
    } else if flags & FIRST_WALL_BREACHED == 0 {
        (CELL_DISCOVERY, 0.12, true)
    } else if flags & GRAY_FOX_RESCUED == 0 {
        (BREAKTHROUGH, 0.42, false)
    } else if flags & EXIT_WALL_BREACHED == 0 {
        (REVELATION, 0.18, true)
    } else if flags & LEFT_PRISON == 0 {
        (ESCAPE_PRESSURE, 0.76, false)
    } else if flags & SLICE_COMPLETED != 0 {
        (RECOVERY, 0.03, true)
    } else if flags & CARD3_RECOVERED != 0 {
        (CHAPTER_EXIT, 0.28, true)
    } else if flags & SHOTMAKER_DEFEATED != 0 {
        (AFTERMATH_SEARCH, 0.10, true)
    } else if flags & GEAR_RECOVERED != 0 {
        (SHOTMAKER_FIGHT, 0.93, false)
    } else if flags & SHOTMAKER_ENCOUNTERED != 0 {
        (SHOTMAKER_RECOVERY_PUZZLE, 0.86, false)
    } else {
        (SHOTMAKER_THREAT, 0.49, true)
    };
    let since = if seconds_without_progress.is_finite() {
        seconds_without_progress.max(0.0)
    } else { 0.0 };
    // Helpful when exploring or searching after a victory, never over
    // actual hostile contact or during the active armed firefight.
    let hint = if under_alert || phase == RECOVERY || phase == SHOTMAKER_FIGHT {
        0
    } else if since >= STRONG_HINT_AFTER_SECONDS {
        2
    } else if since >= SOFT_HINT_AFTER_SECONDS {
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
        assert_eq!(evaluate(state, 27.9, false).unwrap().hint_level, 0);
        assert_eq!(evaluate(state, 28.0, false).unwrap().hint_level, 1);
        assert_eq!(evaluate(state, 74.9, false).unwrap().hint_level, 1);
        assert_eq!(evaluate(state, 75.0, false).unwrap().hint_level, 2);
        assert_eq!(evaluate(state, 100.0, true).unwrap().hint_level, 0);
    }

    #[test] fn there_is_no_blocking_wait_in_pacing_model() {
        let state = CAPTURED | EQUIPMENT_CONFISCATED;
        assert!(evaluate(state, 0.0, false).is_some());
        // The actual action transition remains legal at 0 seconds.
        assert!(crate::gray_fox::transition(
            state, crate::gray_fox::ACTION_BREACH_FIRST_WALL).is_some());
    }

    #[test] fn opening_recovery_and_card_search_are_distinct_beats() {
        let clue = DETENTION_INTELLIGENCE;
        assert_eq!(evaluate(0, 0.0, false).unwrap().phase, CAPTURE_BEAT);
        assert_eq!(evaluate(clue, 0.0, false).unwrap().phase, DETENTION_APPROACH);
        let escaped = CAPTURED | EQUIPMENT_CONFISCATED | FIRST_WALL_BREACHED |
            GRAY_FOX_RESCUED | EXIT_WALL_BREACHED | LEFT_PRISON;
        assert_eq!(evaluate(escaped, 0.0, false).unwrap().phase, SHOTMAKER_THREAT);
        let entered = escaped | SHOTMAKER_ENCOUNTERED;
        assert_eq!(evaluate(entered, 0.0, false).unwrap().phase, SHOTMAKER_RECOVERY_PUZZLE);
        let armed = entered | GEAR_RECOVERED;
        assert_eq!(evaluate(armed, 0.0, false).unwrap().phase, SHOTMAKER_FIGHT);
        let won = armed | SHOTMAKER_DEFEATED;
        assert_eq!(evaluate(won, 0.0, false).unwrap().phase, AFTERMATH_SEARCH);
        assert_eq!(evaluate(won | CARD3_RECOVERED, 0.0, false).unwrap().phase, CHAPTER_EXIT);
        assert_eq!(evaluate(won | CARD3_RECOVERED | SLICE_COMPLETED, 0.0, false).unwrap().phase, RECOVERY);
        assert!(evaluate(won, 0.0, false).unwrap().target_intensity
            < evaluate(armed, 0.0, false).unwrap().target_intensity);
    }

    #[test] fn quiet_after_boss_can_help_and_fighting_never_shows_a_hint() {
        let escaped = CAPTURED | EQUIPMENT_CONFISCATED | FIRST_WALL_BREACHED |
            GRAY_FOX_RESCUED | EXIT_WALL_BREACHED | LEFT_PRISON;
        let armed = escaped | SHOTMAKER_ENCOUNTERED | GEAR_RECOVERED;
        assert_eq!(evaluate(armed, 150.0, false).unwrap().hint_level, 0);
        let won = armed | SHOTMAKER_DEFEATED;
        assert_eq!(evaluate(won, 75.0, false).unwrap().hint_level, 2);
        assert_eq!(evaluate(won, 75.0, true).unwrap().hint_level, 0);
        assert_eq!(evaluate(0, 75.0, false).unwrap().hint_level, 2);
        assert_eq!(evaluate(DETENTION_INTELLIGENCE, 75.0, false).unwrap().hint_level, 2);
    }

    #[test] fn invalid_state_and_pointer_are_rejected() {
        assert!(evaluate(1<<20, 0.0, false).is_none());
        assert_eq!(unsafe {
            mg_gray_fox_pacing(0, 0.0, 0, std::ptr::null_mut())
        }, 0);
    }
}
