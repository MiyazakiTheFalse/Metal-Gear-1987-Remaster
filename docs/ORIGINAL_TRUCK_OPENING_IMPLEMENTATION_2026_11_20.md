# MSX2 Building 1 supply-truck conversion — implementation record (20 November 2026)

**Status:** source and simulated-browser regression verified; **not UE5.7 compiled, imported, packaged or player-tested**. Development branch: `feature/character-art-pipeline-1995` (draft PR #3).

## Player-facing purpose

The 1987 first floor creates the franchise's foundational loop: cautiously inspect three supply trucks, distinguish actual loot from scenery, earn useful rewards and carry knowledge/resources forward. The 3D remake keeps three separately accessible trucks with physical cargo props and a guard near the approach rather than replacing source rewards with a menu grant or an objective checklist. This geometry is a **compressed, reversible remake prototype**, not verified MSX2 exact room topology.

- **West:** single ration. Player can keep it for later; actual use while wounded restores 45 health (clamped at 100). A consumed ration does not regenerate on save/reload.
- **Centre:** Card 1. Real collectible and saved inventory possession; the original full Card-1-gated areas remain later implementation work. Card ownership is not falsely equated with a completed third-floor map.
- **East:** binoculars. Equipping and using them changes camera FOV for optical scouting, without magically discovering enemy positions or mutating guard knowledge. Normal movement and detection rules remain in effect.

**Softlock protection:** capture is not an irreversible loss of the opening route. After actual gear recovery, the west prison service shutter physically opens in both engines; the player can walk back to the receiving yard, collect any missed truck loot and save/reload in that space. The door remains shut during captivity. This return passage is a reversible remake interpretation, not a claim about exact original MSX2 geometry.

All three loot interactions are physical reach/obstruction checks. Each prompts the player locally, gives a short pickup response, removes the world prop and persists ownership. During custody, items are confiscated from use, not erased from inventory; recovery restores the previously earned state. The original Card-2 third-floor persistent cardboard box is **not** moved to the trucks.

## Technical changes

- `browser/game.js` adds west/east item geometry, pickups, inventory/use/zoom, per-item saves, transient scope reset, readable HUD interactions and matching optical FOV in optional rig overlay. Existing Card 1 reward is preserved. Browser save format remains **V10**, additive fields defaulting unearned on older saves.
- Unreal `AMGGrayFoxMission` now handles each of the three already-placed pickup props and records item state. `UMGEquipmentComponent` starts without free test rations, tracks binoculars and supports finite item-count persistence. `AMGPlayerCharacter` toggles binocular FOV and commits ration consumption to the save manager.
- Unreal SaveGame moves from **V6 to V7**. V1–V6 schema recognition remains, ration quantity is persisted separately from the earned flag, and a V7 supply-only checkpoint is accepted before handgun/intelligence/capture. Unknown schemas and impossible locations must still be rejected.
- Browser campaign test exercises three rewards, zoom, save/reload, healing and non-respawn; `tools/test_original_truck_pickup_parity.py` adds structural cross-engine source contracts.

## Evidence and still-open production gates

In the authoring environment, a V8-executed copy of the browser's Node campaign test, with simulated DOM/WebGL/fs/assert, passed **17 logged checkpoints**, including the finite-ration and post-recovery return-route regressions. JavaScript syntax compilation passed for changed browser sources. This is **not** a native Node execution, frame capture or external playtest. The source-test file is committed but full Python unittest suite has not been executed. Unreal compilation, interaction geometry, imported GLBs and camera usability are **unverified**.

Before promoting this sequence: run Node/Python suites locally; compile UE5.7, import and bind 22 source GLBs, test each truck entry and visibility from a controller at 720p, confirm exact pickup positions/LOS and save migration from V1–V6, and use novice+veteran playtests to tune discovery without excessive HUD prompting. The complete original Building 1 routes, usable Card-1 doors, gas room, higher floors, full realistic character art, original-specific sound mix and remaining campaign bosses are still future work.
