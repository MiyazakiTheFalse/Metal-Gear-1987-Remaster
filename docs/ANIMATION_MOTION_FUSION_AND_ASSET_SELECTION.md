# ASSET DECISIONS AND MOVEMENT-FUSION PRODUCTION BIBLE

**19 November 2026 — Binding workflow, not evidence of downloaded/approved final character or animated TX-55.** Governed by Doctrine 000, 004 and 005, Solid Snake protagonist bible, cast dossiers and TX-55 boss bible.

## The target: one believable performer from many lawful sources

Use the best appropriate **movement** from different libraries, and the best appropriate head/body/costume/texture components from lawful art sources. But finished Snake must remain one body with consistent acceleration, weight, foot contact, 1995 silhouette and hand–gun interaction. Guards need disciplined patrol/attention identities; Gray Fox is physically fatigued; Shoot Gunner's shotgun cadence is a gameplay telegraph; Big Boss and Venom have separate production performances.

**Do not equate 'available FBX/GLB' with imported, licensed, rigged and animated character.** No acquired candidate in this repository has yet passed in-editor approval.

## Non-negotiable acceptance gates

1. **Permission and provenance:** author, exact archive filename, version, SHA-256, link and applicable redistribution licence recorded before use. No ripped game meshes, voices, motion data or unofficial model dumps without clear reuse rights.
2. **Canonical silhouette and timeframe:** original 1995 Snake and human Fox recognition before fidelity comparisons. MGS4/MGSV may guide *craft*, not inject 2005/2014 wardrobe.
3. **Rest pose/rig:** bone hierarchy, coordinate system, proportions, handedness, hip/root, scale, retarget poses and contact joints audited. **Hand, muzzle, cover and IK sockets** need names and transform tests.
4. **Input-to-pose fidelity:** in-place gameplay locomotion follows Unreal capsule/AI movement; root-motion performances only where authored and reconciled with collision, nav and mission rules. Never let selected motion clip teleport Snake or grant unintended cover.
5. **Motion-fusion diagnostics:** assess locomotion starts/stops, hard pivots, eight-way walk/run, backward strafe, crouch, prone, cover, transitions, weapon aim/recoil, melee and turn anticipation. Separate motion clips must blend with foot-phase, stride/speed warping and posture consistency; no skating, skeletal pops or joint flips.
6. **Weapon truth:** right/left hand grip, stock, muzzle, aim target, view ray, actual collision muzzle ray, firing animation and recoil **agree**. A cosmetic weapon cannot be visible while confiscated.
7. **Cinematic continuity:** same active mesh/outfit/dirt/props/position in and out of Sequence. Optional face/lip rigs for named speaking cast; idle listening and expressive silence are as important as speech.
8. **Visual/pipeline:** matching PBR light response, normal map axes, proportions, hair/groom budget, readable 720p figure and low/medium/high LOD performance. No polished hero beside visibly cartoonish guards.
9. **Integration and evidence:** full UE5.7 compile, Play-In-Editor and packaged-footage regression including low-spec test hardware, controller, two shoulder cameras, occluded walls, actual VO/captions and skip-safe timeline.
10. **Honest sign-off:** candidate → sourced → imported → integration-tested → player-tested. No status leap allowed, and all gates explicitly documented.

## Candidate source map, with no downloads falsely claimed

- Quaternius universal rig: CC0 page candidate for guard baseline; **not** an authentic Solid Snake face.
- Quaternius animations: CC0 page candidate for locomotion. Inspect exact 2026 release and file format; GLB and FBX are not interchangeable through a single importer.
- KayKit animations: CC0 motion candidate, visual art style not our production standard.
- Epic Game Animation Sample: retarget, pose matching, modern motion and Unreal pipeline reference; verify exact terms and host cost.
- Mixamo: supplemental motion and posing; royalty-free game usage with restrictions on unmodified library redistribution.
- Poly Haven and ambientCG: CC0 texture source candidates, not military costumes or character rigs.
- For TX-55, **author a purpose-built, distinctly TX-55 industrial assembly** rather than borrowing a later Metal Gear's mesh.

The source and missing-evidence states are in ../data/asset_source_quality_gates.json. This is a **source shortlist**, not acquisition or installation.

## Decision discipline

Any proposed asset gets a 0–5 evidence score for **canon silhouette (25%), rig/attachments (20%), foot/contact and motion (20%), world-wide style (15%), performance (10%), permission provenance (10%)**. A beautiful candidate still fails if permission, identity, physical contacts, the exact rig or real UE import cannot be proved. Unverified fields cannot receive assumed maximum scores. This system prevents the 'looks good in screenshot' shortcut.

**Mix-and-match practice:** pick a high-quality locomotion foundation; assign action clips to explicit body-space intent; retarget to ONE master character skeleton; normalise root and scale; establish start/stop transition conditions; tune collision, audio, camera, IK and movement as a unit; then compare the entire ensemble under actual prison and boss lighting. The clip provenance is not the player's concern, but discontinuities are.

## Tools and defensible responsibilities

- Blender or equivalent DCC: inspect/clean source, author costume, fit silhouette, define retarget/rest poses, add hand/muzzle/foot markers, bake material/LODs.
- Unreal Engine 5.7: skeletal and animation import, IK retargeting, BlendSpace/pose-search/AnimBP, sequence camera, physical animation, full playable QA.
- Python stdlib tools in this repo: track source quality and failed gates before merge. **No paid GitHub Actions requirement.**
- Original MSX2 visuals and the actual available J2ME archive: source-truth comparison; later MGS games: quality/motion grammar references only.

## Next measurable proof

Acquire one *legally usable* full-body human baseline and one motion library. Demonstrate in a captured UE5.7 build: start/walk/stop/pivot/crouch/prone/aim/firing with zero visible snap, hand/muzzle alignment, real guard line of sight and responsive input. Only then approve expansion to all cast or label a high-fidelity character complete.
