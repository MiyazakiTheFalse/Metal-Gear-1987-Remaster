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

pub const INTEL: u32 = 64;
pub const RECOVERED: u32 = 128;
pub const SHOTMAKER_DOWN: u32 = 256;
pub const COMPLETE: u32 = 512;
pub const ALL_FLAGS: u32 = 1023;
const STEPS: [(u32, u32); 9] = [
    (6, INTEL), (ACTION_CAPTURE, CAPTURED | EQUIPMENT_CONFISCATED),
    (ACTION_BREACH_FIRST_WALL, FIRST_WALL_BREACHED),
    (ACTION_RESCUE_GRAY_FOX, GRAY_FOX_RESCUED),
    (ACTION_BREACH_EXIT_WALL, EXIT_WALL_BREACHED), (ACTION_ESCAPE, LEFT_PRISON),
    (7, RECOVERED), (8, SHOTMAKER_DOWN), (9, COMPLETE),
];

pub fn transition(flags: u32, action: u32) -> Option<u32> {
    let mut prefix = 0;
    for (expected, grant) in STEPS {
        if prefix == flags && expected == action { return Some(prefix | grant); }
        prefix |= grant;
    }
    None
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
        for (action, _) in STEPS {
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
