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

## Binding 1995 wardrobe guide — separate character identities

### Solid Snake: Operation Intrude N313 (1995)
- **Established:** novice FOXHOUND operative infiltrating Outer Heaven in 1995. Original MSX2 character sprite uses a limited green/blue military palette; contemporary promotional artwork depicts practical green fatigues and field webbing. See Konami original-game history: https://www.konami.com/mg/history/jp/ja/mg and original MG MSX2 cover art.
- **Permitted 3D interpretation:** olive/green field uniform, functional boots/gloves, modest period-plausible pouches and belt, appropriate 1995 hair/face. Recognisable young Snake silhouette. Wear and damage evolve through infiltration and capture.
- **Not source-verified:** precise jacket cut, helmet/headband, armour plate count, 1995-specific weapon harness and backpack layout. These must be labelled design interpretations pending comparison with original MSX2 and supplied J2ME sprites.
- **Do not import as 1995 canon:** 2005 Shadow Moses thermal sneaking suit, 2007 Tanker suit, 2009 Pliskin disguise, 2014 OctoCamo/Solid Eye or Venom Snake's Diamond Dogs hardware.
- **Capture:** retain clothing silhouette but remove equipment visibly. Confiscation should not magically swap Snake to a later sneaking suit.

### Gray Fox: imprisoned agent (1995)
- **Established in the MSX2 release:** a blue recolour of the ordinary prisoner sprite, whose clothing is brown. Comparative evidence: https://comicvine.gamespot.com/grey-fox/4005-47947/ and https://metalgear.fandom.com/wiki/Prisoner_of_war
- **Permitted 3D interpretation:** distinct blue detention uniform (simple institutional garments), tired posture and restrained equipment; expressive rescue reactions. He is experienced and competent but physically compromised.
- **Not source-verified:** exact buttons, fabric weave, collar, footwear, prison identification marks, hair colour in the selected Connect2Media J2ME edition. Verify target sprite and do not silently copy conjectural details.
- **Never depict:** 2005 Cyborg Ninja exoskeleton/helmet, later Zanzibar Land combat gear or a generic fully armed guard body as his detention outfit.
- **Animation requirements:** separate fatigue, guarded walk, listening and recovery sets; his distinct silhouette must remain readable beside Snake under dark prison lighting.

### Solid Snake recognisability — binding acceptance gate

The player must be able to identify the controlled character as **Solid Snake** without relying on a generic military body with an olive tint. **Gameplay identity, distinctive character art, and framing are all required.**

- Commission/author a **distinct 1995 Solid Snake head, hair, facial proportions and outfit silhouette** from verified early-series reference art; no generic soldier body or only palette-swapped guard is acceptable as the final hero asset. His exact 1995 likeness is a source-led reconstruction, not a transplanted MGS4 Old Snake face/suit.
- **Show the face and silhouette** in a brief authored capture opening, close conversational framing and an inventory/Codec portrait. Regular third-person camera should preserve recognisable posture, uniform and hairstyle without compromising control.
- **Explicit identification:** the opening mission briefing, subtitle speaker labels and Codec UI should clearly identify him as **SOLID SNAKE**. His prison outfit is the same operative visibly stripped of equipment, not an unrelated playable character swap.
- **Animation identity:** an experienced younger Snake's guarded, economical movement and combat should read differently from a generic sentry. Keep CQC interpretations consistent with 1995 continuity.
- **Acceptance:** a first-time player must correctly identify Snake in a neutral 3/4 character view, in normal third-person motion, and in the opening capture cutscene; not just from reading a nameplate. Verify at 720p and under intended prison lighting.
- **Production status:** character identification is a hard quality gate, **not yet delivered**. The existing generic soldier skeleton/colour variants are placeholders pending a unique rigged head and costume mesh.

### Cross-game visual guidance

Use MGS4 as **presentation/animation quality benchmark**, never as an excuse to transpose 2014 equipment into 1995. Maintain explicit variant IDs `snake_mg1_1995_infiltration`, `snake_mg1_1995_disarmed` and `gray_fox_mg1_1995_prisoner`. A generic soldier model is a *rigging/animation scaffold*, not a finished character likeness. Treat the MSX2 prisoner blue as comparative evidence, not verification of the target J2ME sprite.

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



## 19 November 2026 — motion-fusion and approvals update

The [asset-selection/motion-fusion bible](ANIMATION_MOTION_FUSION_AND_ASSET_SELECTION.md) and [source-quality manifest](../data/asset_source_quality_gates.json) now govern mixed source motions, physical gun/hand/contact checks, provenance and misleading completion claims. **None of the above candidates has been downloaded, imported or approved as final game art in this repository.** Run the standard-library validator using: python tools/validate_asset_selection.py .
