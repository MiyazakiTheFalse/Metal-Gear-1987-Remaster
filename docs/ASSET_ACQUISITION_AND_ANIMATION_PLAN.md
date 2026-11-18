# Gray Fox Slice — Reusable Art and Animation Acquisition Plan

Updated 2026-11-18. This is a source-verified *candidate* pipeline, NOT a claim that the linked packs are installed, redistributed or suitable as-is for Metal Gear.

## Design and legal constraints

- Preserve the 1987 Outer Heaven / 1995 story, not 1984 Diamond Dogs architecture.
- Never blend ripped Konami meshes/voice recordings into the public repo without redistribution permission. Even artist-authored fan Snake models need the creator's licence and underlying IP considerations reviewed.
- Prefer original character likenesses and custom outfit builds anchored in source comparison; use independent skeletons and licensed animations as motion foundations.
- All downloaded assets require their **individual file/package licence and release version** in a provenance record. A licence on a creator's profile is not proof that all packs use it.
- World scale and joint positions must match the 178 cm later-game reference *provisionally* and the verified J2ME room topology once decoded. Integrating an asset must not change doors or gameplay-critical space.

## Candidates

| Source | Intended role | Licence / conditions | Integration status |
|---|---|---|---|
| [Quaternius Universal Base Characters](https://quaternius.com/packs/universalbasecharacters.html) | Humanoid skeleton baseline for guards/actor proportions | Pack page reports CC0; verify the downloaded release | Candidate only |
| [Quaternius Universal Animation Library](https://quaternius.com/packs/universalanimationlibrary.html) | Locomotion and blend transitions | Pack page reports CC0; verify exact files | Candidate only |
| [KayKit Character Animations](https://kaylousberg.itch.io/kaykit-character-animations) | Additional humanoid animation coverage | Pack page says CC0 | Candidate only |
| [Poly Haven](https://polyhaven.com/license) | Industrial concrete, metal, tiling material maps and lighting | CC0 | Candidate only |
| [ambientCG](https://docs.ambientcg.com/license/) | More PBR material options for damaged concrete and metal | CC0 | Candidate only |
| [Mixamo](https://helpx.adobe.com/creative-cloud/faq/mixamo-faq.html) | Optional retargetable placeholder animations | Royalty free for games; no resale/distribution of raw library as standalone files | Candidate only |
| [Epic Game Animation Sample](https://dev.epicgames.com/documentation/unreal-engine/game-animation-sample-project-in-unreal-engine) | Unreal motion-matching and responsive locomotion reference | Review Fab license and Unreal-specific terms | Candidate only |

## Technical integration order

1. Acquire properly licensed skeleton/base mesh and record file hashes, name, release, URL, licence and limits.
2. Retarget walking / stopping / turning / strafing; then crouch and prone; then cover, melee, captive reactions and breakthrough events.
3. Build original period-appropriate uniforms, gear and recognizable character silhouettes. Avoid obvious asset-pack mismatches in art direction or polygon density.
4. Add root-motion-safe animation transitions, foot lock/IK, animation-notify impact frames, procedural look targets and a reproducible pose test suite.
5. Only after gameplay and proportions are signed off: import 1–2K-scale tiling PBR materials, trim-sheet props, 3D decals, broken concrete meshes and soft/alert lighting.
6. Couple surface material to footstep sound and evidence clues; make wall-break rubble, dust, audio and guard investigations consistent.
7. Profile frame pacing and VRAM on modest PCs. Reduce texture and mesh LOD according to screen coverage; do not default everything to 4K.
8. Save screenshots/video for each stance, animated transition, camera obstruction, cutscene escape and camera collision.

## Current delivered comparison

The browser-only **v0.3 proof-of-play** was generated in the active conversation. It replaces cube-only character proxies with original rounded sphere/cylinder/cube joint chains, movement/crouch blending and punch posing. It also adds plausible prison dressing, procedural textures, persistent rubble and adaptive sound layers. **These are not external CC0 packs, proper skeletal rigs or final character art.**

Browser source/output is currently distributed as a separate local conversation ZIP, *not* synchronized with the Unreal repository. Only publish import-ready assets after rights checks and engine tests. The local WebGL graphical output could not be validated in this environment due to browser restrictions; scripted rescue and Xbox B/camera regression tests passed.

## Acceptance: never mark finished based on visual claims alone

- Matching stylization and source-appropriate 1995 Snake/Gray Fox/guard proportions, with documented source rights.
- Smooth 8-way locomotion, crouch, prone, aim, cover, punch/KO, dialogue and scripted interaction.
- No sliding feet, animation-capsule desync, hand-wall penetration or camera crossing walls.
- Source-faithful architecture, spatial ambience and guard reactions.
- Xbox/keyboard playtesting and low-end Windows performance evidence.

