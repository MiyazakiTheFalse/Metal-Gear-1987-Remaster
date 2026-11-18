//! Gray Fox rescue transaction logic. Engine-neutral, deterministic, no UI effects.
//! This is a playable vertical-slice adaptation of a comparative MSX2 sequence.
//! Exact Connect2Media J2ME room/event triggers are NOT verified.
pub const CAPTURED: u32 = 1 << 0;
pub const EQUIPMENT_CONFISCATED: u32 = 1 << 1;
pub const FIRST_WALL_BREACHED: u32 = 1 << 2;
pub const GRAY_FOX_RESCUED: u32 = 1 << 3;
pub const EXIT_WALL_BREACHED: u32 = 1 << 4;
pub const LEFT_PRISON: u32 = 1 << 5;

pub const ACTION_CAPTURE: u32 = 1;
pub const ACTION_BREACH_FIRST_WALL: u32 = 2;
pub const ACTION_RESCUE_GRAY_FOX: u32 = 3;
pub const ACTION_BREACH_EXIT_WALL: u32 = 4;
pub const ACTION_ESCAPE: u32 = 5;

pub const ALL_FLAGS: u32 = CAPTURED | EQUIPMENT_CONFISCATED
    | FIRST_WALL_BREACHED | GRAY_FOX_RESCUED
    | EXIT_WALL_BREACHED | LEFT_PRISON;

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
        for action in 1..=5 {
            let next = transition(state, action).expect("step must be valid");
            assert_ne!(state, next);
            assert!(transition(next, action).is_none(), "replay must be rejected");
            state = next;
        }
        assert_eq!(state, ALL_FLAGS);
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
