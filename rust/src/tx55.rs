//! Source-faithful TX-55 sabotage rules. Deterministic; no UE, art or audio dependency.
//! Based on original MSX2 guides; J2ME edition parity remains unverified.
//! The gameplay/inventory owner must apply the returned charge count atomically.
//! Cinematic systems only OBSERVE accepted state changes, and never author them.

/// The LAST choice is deliberately not included in Snake's journal.
pub const DIRECTIONS: [u32; 16] = [
    RIGHT, RIGHT, LEFT, RIGHT, LEFT, LEFT, RIGHT, LEFT,
    LEFT, RIGHT, RIGHT, LEFT, RIGHT, LEFT, RIGHT, RIGHT
];
pub const LEFT: u32 = 0;
pub const RIGHT: u32 = 1;
pub const KNOWN_TO_SNAKE: usize = 15;
pub const MAX_CHARGES: u32 = 64;

/// C-compatible state. Owner must save this with campaign snapshot, not presentation time.
/// A 16/16 state is necessarily destroyed; no separate boss health-bar representation.
#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct Tx55State {
    pub accepted_placements: u32,
    pub charges_available: u32,
    pub destroyed: u32,
}

#[repr(u32)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum PlaceResult {
    Invalid = 0,
    Accepted = 1,
    WrongLeg = 2,
    NoCharge = 3,
    StaleEvent = 4,
    AlreadyDestroyed = 5,
}

pub fn valid(state: Tx55State) -> bool {
    state.accepted_placements <= DIRECTIONS.len() as u32
        && state.destroyed <= 1
        && (state.destroyed != 0) == (state.accepted_placements == 16)
        && state.charges_available <= MAX_CHARGES
}

/// The caller supplies expected_progress from when the interaction started.
/// A duplicate/late animation notify cannot plant a second charge, even on
/// consecutive same-leg steps, because it carries a stale progress index.
pub fn place(state: Tx55State, leg: u32, expected_progress: u32)
    -> Result<Tx55State, PlaceResult>
{
    if !valid(state) || leg > RIGHT { return Err(PlaceResult::Invalid); }
    if state.destroyed != 0 { return Err(PlaceResult::AlreadyDestroyed); }
    if expected_progress != state.accepted_placements {
        return Err(PlaceResult::StaleEvent);
    }
    if state.charges_available == 0 { return Err(PlaceResult::NoCharge); }
    if DIRECTIONS[state.accepted_placements as usize] != leg {
        return Err(PlaceResult::WrongLeg);
    }
    let progress = state.accepted_placements + 1;
    Ok(Tx55State {
        accepted_placements: progress,
        charges_available: state.charges_available - 1,
        destroyed: (progress == DIRECTIONS.len() as u32) as u32,
    })
}

/// Only called by a validated physical pickup or checkpoint recovery, never
/// by a cinematic timeline or passive time update.
pub fn collect_charges(state: Tx55State, acquired: u32) -> Option<Tx55State> {
    if !valid(state) || state.destroyed != 0 || acquired == 0 { return None; }
    let mut out = state;
    out.charges_available = out.charges_available.saturating_add(acquired).min(MAX_CHARGES);
    Some(out)
}

/// C ABI return codes match PlaceResult. Writes output only when accepted.
/// # Safety
/// Input must be a readable Tx55State pointer and output writable/aligned.
#[no_mangle]
pub unsafe extern "C" fn mg_tx55_place_charge(
    input: *const Tx55State, leg: u32, expected_progress: u32, output: *mut Tx55State
) -> u32 {
    if input.is_null() || output.is_null() { return PlaceResult::Invalid as u32; }
    let snapshot = unsafe { std::ptr::read(input) };
    match place(snapshot, leg, expected_progress) {
        Ok(after) => {
            unsafe { std::ptr::write(output, after) };
            PlaceResult::Accepted as u32
        }
        Err(reason) => reason as u32,
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn all_sixteen_placements_destroy_and_consume_exactly_sixteen() {
        let mut s = Tx55State { accepted_placements: 0, charges_available: 16, destroyed: 0 };
        for (i, &direction) in DIRECTIONS.iter().enumerate() {
            let before = s;
            s = place(s, direction, i as u32).expect("canonical leg must work");
            assert_eq!(s.accepted_placements, i as u32 + 1);
            assert_eq!(s.charges_available, before.charges_available - 1);
            assert_eq!(s.destroyed, (i == 15) as u32);
        }
        assert_eq!(s.charges_available, 0);
        assert_eq!(place(s, RIGHT, 16), Err(PlaceResult::AlreadyDestroyed));
    }
    #[test]
    fn final_direction_not_disclosed_in_source_intelligence() {
        assert_eq!(KNOWN_TO_SNAKE, 15);
        assert_eq!(DIRECTIONS.len(), 16);
    }
    #[test]
    fn wrong_leg_never_consumes_a_charge_or_changes_progress() {
        let s = Tx55State { accepted_placements: 0, charges_available: 16, destroyed: 0 };
        assert_eq!(place(s, LEFT, 0), Err(PlaceResult::WrongLeg));
        assert_eq!(s.charges_available, 16);
    }
    #[test]
    fn duplicate_notify_cannot_plant_second_same_leg_charge() {
        let s = Tx55State { accepted_placements: 0, charges_available: 16, destroyed: 0 };
        let first = place(s, RIGHT, 0).unwrap();
        assert_eq!(DIRECTIONS[0], DIRECTIONS[1]);
        assert_eq!(place(first, RIGHT, 0), Err(PlaceResult::StaleEvent));
        assert_eq!(place(first, RIGHT, 1).unwrap().accepted_placements, 2);
    }
    #[test]
    fn zero_ammo_no_soft_advance_and_recovery_pickup() {
        let s = Tx55State::default();
        assert_eq!(place(s, RIGHT, 0), Err(PlaceResult::NoCharge));
        let recovered = collect_charges(s, 18).unwrap();
        assert_eq!(recovered.charges_available, 18);
        assert!(place(recovered, RIGHT, 0).is_ok());
        assert_eq!(collect_charges(recovered, 0), None);
    }
    #[test]
    fn corrupted_save_is_rejected_without_mutating_output() {
        let bad = Tx55State { accepted_placements: 16, charges_available: 10, destroyed: 0 };
        assert!(!valid(bad));
        assert_eq!(place(bad, RIGHT, 16), Err(PlaceResult::Invalid));
        let mut out = Tx55State { accepted_placements: 4, charges_available: 11, destroyed: 0 };
        let status = unsafe { mg_tx55_place_charge(&bad, RIGHT, 16, &mut out) };
        assert_eq!(status, PlaceResult::Invalid as u32);
        assert_eq!(out.accepted_placements, 4);
    }
    #[test]
    fn ffi_rejects_null_and_keeps_output_unchanged() {
        let mut out = Tx55State::default();
        let status = unsafe { mg_tx55_place_charge(std::ptr::null(), RIGHT, 0, &mut out) };
        assert_eq!(status, PlaceResult::Invalid as u32);
        assert_eq!(out, Tx55State::default());
    }
    #[test]
    fn charge_pickups_bounded_and_never_restart_finished_boss() {
        let s = Tx55State { accepted_placements: 0, charges_available: 62, destroyed: 0 };
        assert_eq!(collect_charges(s, 999).unwrap().charges_available, MAX_CHARGES);
        let done = Tx55State { accepted_placements: 16, charges_available: 0, destroyed: 1 };
        assert_eq!(collect_charges(done, 12), None);
    }
}
