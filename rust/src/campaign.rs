//! Engine-neutral 1995 narrative chapter transition policy.
//! The original J2ME story event ordering has not been traced. This is a
//! conservative remake progression contract: a player must consciously enter
//! capture after helping a prisoner, then free Fox and reclaim confiscated gear.
pub const OPENING: u32 = 0;
pub const FOX_PRISON: u32 = 1;
pub const POST_RESCUE: u32 = 2;

pub const FIRST_PRISONER_INTEL: u32 = 1 << 0;
pub const GRAY_FOX_REPORT: u32 = 1 << 1;
pub const GEAR_RECOVERED: u32 = 1 << 2;
pub const PRISON_EXIT_COMPLETE: u32 = 1 << 3;
pub const KNOWN_EVENTS: u32 =
    FIRST_PRISONER_INTEL | GRAY_FOX_REPORT | GEAR_RECOVERED | PRISON_EXIT_COMPLETE;

/// Check one voluntary chapter transition. Never infer a transition from a
/// timer or dialogue audio finishing. No free skipping from opening to escape.
pub fn advance(current: u32, proposed: u32, earned: u32) -> Option<u32> {
    if earned & !KNOWN_EVENTS != 0 { return None; }
    match (current, proposed) {
        (OPENING, FOX_PRISON) if earned & FIRST_PRISONER_INTEL != 0 =>
            Some(FOX_PRISON),
        (FOX_PRISON, POST_RESCUE)
            if earned & (GRAY_FOX_REPORT | GEAR_RECOVERED | PRISON_EXIT_COMPLETE)
                == (GRAY_FOX_REPORT | GEAR_RECOVERED | PRISON_EXIT_COMPLETE) =>
            Some(POST_RESCUE),
        _ => None,
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn first_pow_clue_unlocks_capture_but_no_radio_is_required() {
        assert_eq!(advance(OPENING, FOX_PRISON, 0), None);
        assert_eq!(advance(OPENING, FOX_PRISON, FIRST_PRISONER_INTEL),
                   Some(FOX_PRISON));
    }
    #[test]
    fn cannot_skip_directly_to_post_rescue() {
        assert_eq!(advance(OPENING, POST_RESCUE, KNOWN_EVENTS), None);
        assert_eq!(advance(FOX_PRISON, POST_RESCUE, GRAY_FOX_REPORT), None);
    }
    #[test]
    fn actual_escape_and_gear_are_required_to_finish() {
        assert_eq!(advance(FOX_PRISON, POST_RESCUE,
            GRAY_FOX_REPORT | PRISON_EXIT_COMPLETE), None);
        assert_eq!(advance(FOX_PRISON, POST_RESCUE,
            GRAY_FOX_REPORT | GEAR_RECOVERED), None);
        assert_eq!(advance(FOX_PRISON, POST_RESCUE,
            GRAY_FOX_REPORT | GEAR_RECOVERED | PRISON_EXIT_COMPLETE),
            Some(POST_RESCUE));
    }
    #[test]
    fn transition_is_idempotent_and_fail_closed() {
        assert_eq!(advance(POST_RESCUE, POST_RESCUE, KNOWN_EVENTS), None);
        assert_eq!(advance(FOX_PRISON, POST_RESCUE, KNOWN_EVENTS | (1 << 29)), None);
        assert_eq!(advance(9, 10, KNOWN_EVENTS), None);
    }
}
