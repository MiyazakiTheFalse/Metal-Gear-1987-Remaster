# Third-person gameplay vertical slice (source milestone)

## Intended feel
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
- Final Snake model, locomotion animation graphs, traversal, vault, prone, adaptive cover and CQC.
- Original room geometry/script import, codec, UI, story and inventory.
- Advanced guard squad coordination, search routes, object interactions, audio and save system.
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
