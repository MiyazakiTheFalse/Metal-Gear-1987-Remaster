//! Gray Fox rescue transaction logic. Engine-neutral, deterministic, no UI effects.
//! This is a playable vertical-slice adaptation of a comparative MSX2 sequence.
//! Exact Connect2Media J2ME room/event triggers are NOT verified.
pub const CAPTURED: u32 = 1 << 0;
pub const EQUIPMENT_CONFISCATED: u32 = 1 << 1;
pub const FIRST_WALL_BREACHED: u32 = 1 << 2;
pub const GRAY_FOX_RESCUED: u32 = 1 << 3;
pub const EXIT_WALL_BREACHED: u32 = 1 << 4;
pub const LEFT_PRISON: u32 = 1 << 5;
pub const SHOTMAKER_ENCOUNTERED: u32 = 1 << 6;
pub const GEAR_RECOVERED: u32 = 1 << 7;
pub const SHOTMAKER_DEFEATED: u32 = 1 << 8;
pub const SLICE_COMPLETED: u32 = 1 << 9;
pub const TRANSMITTER_PLANTED: u32 = 1 << 10;
pub const TRANSMITTER_REMOVED: u32 = 1 << 11;
pub const CARD3_RECOVERED: u32 = 1 << 12;
pub const DETENTION_INTELLIGENCE: u32 = 1 << 13;

pub const ACTION_CAPTURE: u32 = 1;
pub const ACTION_BREACH_FIRST_WALL: u32 = 2;
pub const ACTION_RESCUE_GRAY_FOX: u32 = 3;
pub const ACTION_BREACH_EXIT_WALL: u32 = 4;
pub const ACTION_ESCAPE: u32 = 5;
pub const ACTION_ENTER_SHOTMAKER: u32 = 6;
pub const ACTION_RECOVER_GEAR: u32 = 7;
pub const ACTION_DEFEAT_SHOTMAKER: u32 = 8;
pub const ACTION_COMPLETE_SLICE: u32 = 9;
pub const ACTION_COLLECT_CARD3: u32 = 10;
pub const ACTION_REMOVE_TRANSMITTER: u32 = 11;
pub const ACTION_LEARN_DETENTION_ROUTE: u32 = 12;

pub const ALL_FLAGS: u32 = CAPTURED | EQUIPMENT_CONFISCATED
    | FIRST_WALL_BREACHED | GRAY_FOX_RESCUED
    | EXIT_WALL_BREACHED | LEFT_PRISON | SHOTMAKER_ENCOUNTERED
    | GEAR_RECOVERED | SHOTMAKER_DEFEATED | SLICE_COMPLETED
    | TRANSMITTER_PLANTED | TRANSMITTER_REMOVED
    | CARD3_RECOVERED | DETENTION_INTELLIGENCE;

/// Returns None on repeated actions or skipped mission prerequisites.
/// Other game systems can listen to the accepted state transition for presentation.
pub fn transition(flags: u32, action: u32) -> Option<u32> {
    if flags & !ALL_FLAGS != 0 { return None; }
    let next = match action {
        ACTION_CAPTURE if flags & CAPTURED == 0 =>
            flags | CAPTURED | EQUIPMENT_CONFISCATED,
        ACTION_BREACH_FIRST_WALL if flags & CAPTURED != 0
            && flags & FIRST_WALL_BREACHED == 0 =>
            flags | FIRST_WALL_BREACHED,
        ACTION_RESCUE_GRAY_FOX if flags & FIRST_WALL_BREACHED != 0
            && flags & GRAY_FOX_RESCUED == 0 =>
            flags | GRAY_FOX_RESCUED,
        ACTION_BREACH_EXIT_WALL if flags & GRAY_FOX_RESCUED != 0
            && flags & EXIT_WALL_BREACHED == 0 =>
            flags | EXIT_WALL_BREACHED,
        ACTION_ESCAPE if flags & EXIT_WALL_BREACHED != 0
            && flags & LEFT_PRISON == 0 =>
            flags | LEFT_PRISON,
        ACTION_ENTER_SHOTMAKER if flags & LEFT_PRISON != 0
            && flags & SHOTMAKER_ENCOUNTERED == 0 =>
            flags | SHOTMAKER_ENCOUNTERED,
        ACTION_RECOVER_GEAR if flags & SHOTMAKER_ENCOUNTERED != 0
            && flags & GEAR_RECOVERED == 0 =>
            flags | GEAR_RECOVERED | TRANSMITTER_PLANTED,
        ACTION_DEFEAT_SHOTMAKER if flags & GEAR_RECOVERED != 0
            && flags & SHOTMAKER_DEFEATED == 0 =>
            flags | SHOTMAKER_DEFEATED,
        ACTION_COMPLETE_SLICE if flags & CARD3_RECOVERED != 0
            && flags & SLICE_COMPLETED == 0 =>
            flags | SLICE_COMPLETED,
        ACTION_COLLECT_CARD3 if flags & SHOTMAKER_DEFEATED != 0
            && flags & CARD3_RECOVERED == 0 =>
            flags | CARD3_RECOVERED,
        ACTION_REMOVE_TRANSMITTER if flags & TRANSMITTER_PLANTED != 0
            && flags & TRANSMITTER_REMOVED == 0 =>
            flags | TRANSMITTER_REMOVED,
        // Can occur before capture in the expanded opening; older prison-only
        // saves can still resume from the legacy capture checkpoint.
        ACTION_LEARN_DETENTION_ROUTE if flags & CAPTURED == 0
            && flags & DETENTION_INTELLIGENCE == 0 =>
            flags | DETENTION_INTELLIGENCE,
        _ => return None,
    };
    Some(next)
}

/// Version 1 C ABI. Returns 1 on accepted transition; 0 on invalid/repeated.
/// # Safety
/// Caller must provide a valid, writable 32-bit aligned output pointer.
#[no_mangle]
pub unsafe extern "C" fn mg_gray_fox_transition(
    flags: u32, action: u32, out_flags: *mut u32
) -> u32 {
    if out_flags.is_null() { return 0; }
    match transition(flags, action) {
        Some(next) => {
            unsafe { std::ptr::write(out_flags, next); }
            1
        }
        None => 0,
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn only_correct_sequence_completes_rescue() {
        let mut state = 0;
        for action in [12, 1, 2, 3, 4, 5, 6, 7, 11, 8, 10, 9] {
            let next = transition(state, action).expect("step must be valid");
            assert_ne!(state, next);
            assert!(transition(next, action).is_none(), "replay must be rejected");
            state = next;
        }
        assert_eq!(state, ALL_FLAGS);
    }
    #[test]
    fn cannot_skip_shotmaker_puzzle() {
        let rescued = CAPTURED | EQUIPMENT_CONFISCATED | FIRST_WALL_BREACHED |
            GRAY_FOX_RESCUED | EXIT_WALL_BREACHED | LEFT_PRISON;
        assert!(transition(rescued, ACTION_RECOVER_GEAR).is_none());
        assert!(transition(rescued, ACTION_DEFEAT_SHOTMAKER).is_none());
        let encountered = transition(rescued, ACTION_ENTER_SHOTMAKER).unwrap();
        assert!(transition(encountered, ACTION_DEFEAT_SHOTMAKER).is_none());
        let gear = transition(encountered, ACTION_RECOVER_GEAR).unwrap();
        assert!(transition(gear, ACTION_COMPLETE_SLICE).is_none());
    }
    #[test]
    fn former_5_step_checkpoint_is_still_accepted() {
        let mut state = 0;
        for a in 1..=5 { state = transition(state, a).unwrap(); }
        assert_eq!(state, 63);
        assert!(transition(state, ACTION_ENTER_SHOTMAKER).is_some());
    }
    #[test]
    fn transmitter_must_be_discovered_before_removal_and_is_not_gear_pickup() {
        let mut f = 0;
        for action in 1..=7 { f = transition(f, action).unwrap(); }
        assert_ne!(f & TRANSMITTER_PLANTED, 0);
        assert_eq!(f & TRANSMITTER_REMOVED, 0);
        let g = transition(f, ACTION_REMOVE_TRANSMITTER).unwrap();
        assert_ne!(g & TRANSMITTER_REMOVED, 0);
        assert!(transition(g, ACTION_REMOVE_TRANSMITTER).is_none());
    }
    #[test]
    fn card_pickup_must_follow_boss_defeat_and_precede_slice_complete() {
        let mut f = 0;
        for action in 1..=7 { f = transition(f, action).unwrap(); }
        assert!(transition(f, ACTION_COLLECT_CARD3).is_none());
        assert!(transition(f, ACTION_COMPLETE_SLICE).is_none());
        f = transition(f, ACTION_DEFEAT_SHOTMAKER).unwrap();
        assert!(transition(f, ACTION_COMPLETE_SLICE).is_none());
        f = transition(f, ACTION_COLLECT_CARD3).unwrap();
        assert!(transition(f, ACTION_COMPLETE_SLICE).is_some());
    }
    #[test]
    fn prisoner_clue_is_distinct_from_legacy_capture_checkpoint() {
        let learned = transition(0, ACTION_LEARN_DETENTION_ROUTE).unwrap();
        assert_ne!(learned & DETENTION_INTELLIGENCE, 0);
        assert_eq!(learned & CAPTURED, 0);
        assert!(transition(learned, ACTION_CAPTURE).is_some());
        // No new prerequisite retroactively invalidates a saved 1995 prison run.
        assert!(transition(0, ACTION_CAPTURE).is_some());
    }
    #[test]
    fn cannot_rescue_without_breach() {
        assert!(transition(0, ACTION_RESCUE_GRAY_FOX).is_none());
        assert!(transition(CAPTURED | EQUIPMENT_CONFISCATED, ACTION_RESCUE_GRAY_FOX).is_none());
    }
    #[test]
    fn cannot_escape_without_rescuing_fox() {
        let state = CAPTURED | EQUIPMENT_CONFISCATED | FIRST_WALL_BREACHED;
        assert!(transition(state, ACTION_BREACH_EXIT_WALL).is_none());
        assert!(transition(state, ACTION_ESCAPE).is_none());
    }
    #[test]
    fn failed_attempt_does_not_mutate_output() {
        let mut out = 77;
        let accepted = unsafe { mg_gray_fox_transition(0, ACTION_ESCAPE, &mut out) };
        assert_eq!(accepted, 0);
        assert_eq!(out, 77);
    }
    #[test]
    fn ffi_null_pointer_fails() {
        assert_eq!(unsafe {
            mg_gray_fox_transition(0, ACTION_CAPTURE, std::ptr::null_mut())
        }, 0);
    }
    #[test]
    fn extraneous_bits_fail_closed() {
        assert!(transition(1 << 30, ACTION_CAPTURE).is_none());
    }
}
