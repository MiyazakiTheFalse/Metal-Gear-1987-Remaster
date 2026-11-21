# Production convergence pass — 20 November 2026
**Branch:** feature/character-art-pipeline-1995 · **Status:** playable browser-source prototype / Unreal source uncompiled / final production art absent.

## This pass actually changed shipped source AND model files

### Character geometry, skeleton and performance
- Regenerated and committed four real skinned glTF 2.0 GLBs: `browser/assets/models/{snake,fox,guard,shoot_gunner}.glb`.
- Each now uses an **18-joint** pelvis/chest/head/arms/hands/legs/feet **plus independent jaw and brow** skeleton; both body and facial mesh geometry follow those joints.
- Animations exported with 49 samples across one loop/action, including existing Idle/Walk/Run/Crouch/Prone/Down and new Aim, Cover, Melee, Reaching, Hit, RadioListen, CrouchAim, ProneAim. Fox retains Captive and Shoot Gunner retains Address/Shoulder/Recoil/Search.
- Final committed binary inventories from export: **Snake 14 clips/662,064 bytes; Fox 15/500,992; Guard 14/517,616; Shoot Gunner 18/614,360.**
- Browser's active field renderer now receives face motion driven by authored dialogue/performance state; 3D mesh and collision still refer to their real world-state positions. This is **not** blendshape/lipsync-quality dialogue or motion capture.
- New `browser/tools/audit_field_glbs.cjs` refuses outdated/invalid model files. Use `node browser/tools/export_field_rigs.cjs` followed by `node browser/tools/audit_field_glbs.cjs`.

**Realism gate is NOT passed.** These are original authored procedural prototype characters, not photorealistic licensed 1995 facial scans, production hair, skin/eyes, costume PBR maps, 4K maps, individually modeled fingers, retargeted full mocap or UE skeletal mesh/AnimBlueprint deliverables. A higher joint count and functioning animations are necessary infrastructure, **not** sufficient artistic quality.

### Whole-campaign playable continuation beyond the prison
- Both codebases now contain a reversible west-lift transition to **Building 1 roof / Hind D** after Machinegun Kid and the parachute pickup.
- Source-authored elevated roof deck (Unreal), blast shelters with physical collision/LOS and a provisional helicopter flyover with patrol → telegraph → firing → evasive windows.
- Player-led grenade launcher counterplay, finite six-round prototype rack and three effective hits to disable the aircraft. These timings/durability quantities are **remake prototype values, not original MSX2-verified figures**; there is no physical grenade projectile simulation or finished helicopter mesh.
- Browser save V9 and Unreal SaveGame V5 retain the new encounter, ammunition and defeated state. Legacy browser saves and Unreal V1–V4 schema remain accepted; Unreal's position validator explicitly rejects fabricated rooftop checkpoints without source-earned gates.
- Both allow withdrawal through the west lift, then re-entry with the same encounter progress; neither routes to Building 2 yet. This stage is not a complete production-ready Hind D encounter.

## Verification performed
- Fresh JavaScript syntax compilation of browser game and rig/export/test scripts: passed.
- Executed field rig generator in an isolated V8 harness: four valid 18-joint meshes and finite facial poses.
- Executed the exporter in a compatible V8 binary harness, validated all four glTF binary headers, declared lengths, skeleton counts and clip sets, committed the generated GLBs, and fetched GitHub blob SHAs to verify that the actual files exist in the branch.
- Executed the entire browser `test_campaign.cjs` regression logic in a mocked V8 harness (equivalent source JS, stubbed DOM/WebGL/Node file reads): **14 logged PASS checkpoints**, including the new roof stage, finite grenade use, checkpoint load, return and re-entry.
- Added Python cross-engine source contracts `tools/test_roof_hind_parity.py` and updated save-schema assertions.

**Not done / must not be claimed:** native Node suite, full Python unittest suite, Rust cargo tests, Unreal 5.7 compile/PIE, packaged game, glTF import into Unreal, real 720p screenshot/character comparison, real controller/audio capture, helicopter motion and collision playtest on production maps, asset licensing audit, specialist animation/art review or external novice playtest.

## Next production pass — acceptance gates
1. **Animation and realistic cast:** Import four actual GLBs to Unreal as skeletal meshes. Verify skeleton/skin/scale/axes, root motion, gait speed/foot sliding, hand-muzzle contact, retargeting, crouch-to-prone transitions, guard carrying/dragging contacts and facial speech. Replace prototype geometry with lawful production-worthy PBR heads and bodies; preserve identifiable 1995 costumes and character bible specifications. Capture multiple 720p in-engine close/mid/far views before accepting a model.
2. **Hind D:** Add real in-world launcher possession/pickup and ballistic projectile/impact, moving rotors, hit zones, warning/counterplay animations, readable cover, original-source encounter order, sky/roof lighting, actual spatial sound and accessible captions. Validate difficulty from cover and deliberately spent ammunition. Make engine combat parity real, not just structurally similar.
3. **Whole game:** Continue the source-verified floor/roof/landing interconnection into Building 2's tank/mines and bulldozer, then Fire Trooper/TX-11, Building 3, Coward Duck, TX-55 and the final confrontation. Preserve backtracking doors, cards, stealth responses, evidence, body persistence and narrative knowledge boundaries.
4. **Build quality:** Run `node browser/tests/test_campaign.cjs`, `node browser/tests/test_field_rigs.cjs`, `node browser/tools/audit_field_glbs.cjs`, `python -m unittest discover -s tools -p 'test_*.py'`, `cargo test` and the UE5.7 Windows build; record failures and only promote an area after actual runtime/playtest evidence.

The project doctrine still applies: a milestone is only complete when its **physical, audiovisual, interactive, stateful and player-tested experience** is coherent. Do not count a source actor, design note or new model file as MGS4-quality production art.
