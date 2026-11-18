# Gray Fox Prison Rescue — First Integrated Vertical Slice

**Status: authored source, Unreal not compiled, no packaged executable.**

## Source truth and boundaries

- Original story reference: *Metal Gear* (1987) MSX2; Gray Fox is held within the Outer Heaven complex and the comparative capture sequence involves cell-wall escape.
- Supplied production target: Connect2Media J2ME v3.3.0 a1. Original J2ME prison event triggers and geometry remain **unverified**. Nothing about the provisional greybox room dimensions is represented as an authentic 1987/2012 specification.
- This mission intentionally **starts at the capture checkpoint** to test the scene in isolation. It is not a claim the original story starts with Snake captured.
- Later games are quality references for free camera, stealth and responsive actions, not a reason to alter core 1995 plot continuity.

## Pacing and mandatory sequence

| Stage | Rust action | Space | Player control | Pressure |
|---|---|---|---|---|
| Capture checkpoint | 1 | Initial cell | Regained immediately | Orientation, vulnerability |
| Breach first wall | 2 | First divider | Press E/gamepad face-left; no wait | A short action peak |
| Meet Gray Fox | 3 | Adjacent cell | Interact within range | Eleven seconds of optional on-screen dialogue, no input lock |
| Breach exit wall | 4 | Second divider | Immediate breach after rescue | Rising tension |
| Escape | 5 | Guarded corridor | Reach far side of lane; bypass patrols | Legible stealth challenge |
| Recovery | State after 5 | Corridor end | Player retains control | Mission completion and checkpoint |

Pacing functions in Rust are *guidance*, never a hidden timer, enemy spawn mandate or mandatory pause. At >=14 seconds of no progress, a subtle hint can appear; at >=35 seconds, a stronger hint. If guards are actively alert, hints are suppressed to reduce HUD clutter. No new guard wave is spawned at the moment Gray Fox is rescued.

## How the prototype implements this

- GameMode spawns \`AMGGrayFoxMission\` in the default runtime scene.
- Player's E / gamepad face-left triggers \`TryInteract\`; interactions require an actual nearby, forward-facing target and a clear line of sight.
- Rust ABI \`mg_gray_fox_transition\` provides ordered, once-only state changes. Unreal C++ has a matching fallback when \`MG_WITH_RUST=0\`; the Rust native lib is preferable.
- The two walls remain solid until their respective events; actor collision is removed only when the state transaction commits.
- Snake begins unarmed. Existing stun is disabled while equipment remains confiscated.
- Gray Fox dialogue is temporary, authored placeholder text; there is no voice, performance animation or complete scene localization yet.
- The escape lane has pre-existing guards, now with solid blocking capsules (they cannot phase through walls). Guard behaviour remains a basic prototype.
- \`UMGGrayFoxSaveGame\` persists bitflags and a checkpoint position after each accepted action. Save/load does not replay one-shot actions.
- F9 deletes the slice checkpoint and reopens the map for replay. This is a developer/test shortcut, not final gameplay UI.
- During early pacing the environment is quiet because it is physically separated from the perimeter, not because we erase a legitimate enemy alert.
- The authored room is a **straight-line functional test**, not a verified exact reconstruction. Original missions/doors/topology still govern the final replacement.

## Intended test cases before calling this playable

1. Compile UE5.7 C++ and Rust. Fix compile/link errors; verify native ABI version in packaged output.
2. Enter cell, free-look and crouch without wall penetration; verify camera probe retracts.
3. Try to rescue Gray Fox from outside the cell: denied. Try to breach exit before rescue: denied.
4. Breach first wall, walk into Gray Fox cell, interact; verify the rescue flag and one-time cue.
5. Interact repeatedly; no duplicate rescue, duplicated cutscene or corrupted inventory.
6. Stand still for >=14s and >=35s per step; verify hints and that E works at 0s too.
7. Try to use stun while equipment confiscated: blocked. Restart and reload save after each beat.
8. Walk into guarded route: guards patrol without moving through walls; use crouch/cover and test detection.
9. Trigger guard alert: hint overlay disappears; no fabricated enemies spawn on rescue.
10. Escape and verify final checkpoint; reset via F9 and repeat with controller.
11. Verify first-contact run timing, camera collision, failure recovery and low-end PC frame pacing.
12. Compare each rebuilt room against exact J2ME data and source screenshots before declaring fidelity.

## Known limitations

No Rust toolchain or Unreal editor is available in this authoring environment. The code and unit tests have been written and checked for structural consistency, **not run in the game engine**. Guard navigation is simple swept straight-line movement, and prototype characters are primitives; AI damage, cinematics, original level art and proper save migration are future work.

## Reference design research

- [Level Design Book — Pacing](https://book.leveldesignbook.com/process/preproduction/pacing)
- [GameDeveloper — Anatomy of a Stealth Encounter](https://www.gamedeveloper.com/design/the-anatomy-of-a-stealth-encounter)
- [TIME — Kojima on stress/tension relief in Metal Gear](https://time.com/2862423/interview-hideo-kojima/)
- [Konami Metal Gear MSX manual](https://metalgear.konami.net/manual/mc1/mg1_msx/pc/en/page02.html)
