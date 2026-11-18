# Third-person gameplay vertical slice (source milestone)

## Intended feel

All source work follows [Doctrine 000 — How We Build](DOCTRINE_000_HOW_WE_BUILD.md), and spatial layout follows [Doctrine 005](DOCTRINE_005_SPATIAL_STRUCTURE.md). Source-only improvements require Unreal compilation and synchronized audio/visual playtesting before acceptance.
Modern, freely controlled third-person camera with quick aim transition and tactical stealth; **not a claim of MGS4/MGSV-equivalent polish yet**. Source-only implementation still requires compilation and playtesting.

## First-pass implemented in source
- Mouse and right-stick camera with camera-relative movement.
- Walk/sprint/crouch with different detection visibility and noise.
- Over-the-shoulder aim with smooth zoom, aim strafing and left/right shoulder swap.
- Camera/character obstruction trace for prototype nonlethal hitscan.
- Two guard patrol actors with Rust line-of-sight, distance, sound and awareness states.
- Cover walls and objects blocking traces.
- HUD with controls and aiming crosshair.
- F6 to select higher graphics presets **only at the player's request**; settings are saved by Unreal.
- Default low-overhead untextured geometry; no Lumen, Nanite or virtual shadows.

## Input map
| Keyboard / mouse | Gamepad | Action |
|---|---|---|
| WASD | Left stick | Move relative to camera |
| Mouse | Right stick | Free look |
| Hold RMB | Hold LT | Over-shoulder aim |
| LMB while aiming | RT while aiming | Prototype nonlethal stun |
| Left Shift | Left stick press | Sprint |
| Left Ctrl | B / Circle | Toggle crouch |
| Q | Right stick press | Swap shoulder |
| F6 | (menu pending) | Cycle Performance / Balanced / High / Ultra |

## Not implemented yet
- Final Snake model, authored locomotion animation graphs, traversal, vault, production-ready prone, rigged wall cover and CQC. Auto wall contact and corner peeking currently exist only as prototype C++ and camera logic.
- Original room geometry/script import, final transceiver portraits and recorded VO, finalized UI, narrative sequences and complete inventory. An optional source-only transceiver text UI and basic mission events now exist.
- Advanced guard squad coordination, NavMesh-aware search routes, complete object interactions, authored spatial audio and full-game saving. Basic source-only sound distraction and Gray Fox checkpoint saving exist.
- Weapon assets / ballistics; current stun is a placeholder.
- User-facing graphics options menu and separately downloadable HD texture pack.
- Windows packaging, performance testing or Switch builds.

## Acceptance criteria for next milestone
1. UE C++ compiles against selected version on a build machine.
2. Packaged low-spec Windows build boots without UE Editor.
3. 360-degree free camera doesn't clip through walls.
4. Aiming transitions and shooting correctly hit or respect cover.
5. Guards independently hear sprinting and see an uncovered player.
6. Crouching, cover and slow movement produce measurably better stealth outcomes.
7. Performance remains selectable as the first-launch default across restarts.
