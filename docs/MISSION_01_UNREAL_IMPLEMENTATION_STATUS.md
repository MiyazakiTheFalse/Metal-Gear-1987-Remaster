# Mission 1 source implementation — Unreal greybox, 21 November 2026

**Implementation status: Unreal C++ source authored, NOT compiled, packaged or gameplay-certified.** The repository currently has no browser runtime source, so a matching browser version was not produced in this pass.

## Implemented (source)

- `AMGRemasterGameMode::StartPlay` starts Snake at the provisional Outer Heaven Building One approach, not in the Gray Fox prison. The prison prototype source remains intact for reactivation through later mission selection.
- `AMGMissionOne` constructs seven floor footprints, segmented perimeter walls, three-truck first group and later truck group, usable first/second group thresholds, blockout cover, a Card 1 door and elevator arrival marker. Two unverified optional wings are sealed. All dimensions are **remake test metres**.
- G01–G05 are physically spawned with authored multi-point waypoint patrols; G06 stays in the blueprint but **does not spawn** while its optional wing is sealed. Existing guard sight, suspicion, investigation and nonlethal responses are reused; structural attenuation reduces movement hearing through walls.
- E/X pickup interactions: first yard ration, Card 1, binoculars; Card 1 room gas mask; later yard handgun without ammunition, land mines. Pickups are placed at reachable edges of proxy truck volumes, not inside solid cubes.
- Inventory begins empty apart from the NONE slots; prototype stun pistol and test ration are unowned for Mission 1. The earlier prison confiscation system remains available.
- Card 1 opens the physically blocking door. Pickup IDs, open door and elevator discovery persist through the `MissionOneV1` Unreal SaveGame slot; collected inventory is reconstructed from saved IDs on restart. Currently checkpoint position deliberately starts at the safe opening to avoid spawning inside unvalidated geometry.
- HUD uses the Mission 1 coordinate bounds and gives stage-aware objectives. Optional transceiver has temporary Big Boss introduction text, not finished VO.
- Source-only regression test: `python -m unittest discover -s tools -p 'test_*.py'`. Contract consistency script: `python tools/validate_mission_01_layout.py`.

## Explicitly not finished

- Exact J2ME first mission room and guard/event trace, verified canonical item coordinates, truck interiors, real military-grade structural detail and source-correct arrival cinematic.
- Building One upper-floor elevator journey; interacting with the lift currently displays a message, not a level transition. No later mission is being represented as complete.
- A tested NavMesh, collision checks against vehicle geometry, guard body discovery, line-of-sight sweeps, full audio propagation or alarms with authentic source scripts.
- Ration consumption and card selection integration beyond existing inventory prototypes; physically correct and rigged guards, service trucks, character animation, VFX and production audio.
- Game-over/restart, completed save continuity for player health and guards, player HUD quality review, packaged Unreal build, and browser parity.

## Known design decisions and acceptance gates

1. **Source-first:** the 1987 MSX2 group-of-three trucks and item sequence are comparative evidence. The specified J2ME 3.3.0 source must be decoded before calling these coordinates source-accurate. Reference: https://thesnakesoup.org/metal-gear-msx-walkthrough/.
2. **Military plausibility:** guards assigned to checkpoint, truck watch, inspection and elevator access, drawn from the public concepts of US Army TC 3-22.6 and ATTP 3-39.32. These are fictionalized duties for stealth readability, not actual military post diagrams.
3. **Never silently bypass Card 1:** the store threshold has floor-to-ceiling adjacent wall segments. Optional paths remain physically sealed during fidelity mode.
4. **No invisible starting gun:** the loadout reset clears the prototype ammunition and item stock. Later ammunition must be added at its source-appropriate upper-floor location.
5. **Review priorities:** Unreal compile, actual spawn on fresh save, reach all six pickups, prove no card-door clipping/bypass, patrol path collision, all-state awareness recovery, save/reload, upper-floor architecture, J2ME comparison, then packaged playtest.
6. **Parity:** when a browser game implementation is added to the repository, both runtimes must use shared source identifiers and the same mission progression/save rules.

### Test walkthrough

Fresh launch → observe G01 without forced combat → enter first truck yard → take Card 1 → unlock store door → retrieve gas mask → cross into second truck yard → obtain empty handgun and optional mines → enter elevator marker → save/restart and verify the pickups do not respawn and Card 1 door stays open. Re-run with no optional equipment and once deliberately detected. Record nav/collision and camera screenshots with both shoulders.

This is an **implementation source pass**, not a completed playable mission or a certified release.
