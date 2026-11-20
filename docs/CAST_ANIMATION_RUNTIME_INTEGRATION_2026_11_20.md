# Character presentation integration pass — 20 November 2026

**Branch:** `feature/character-art-pipeline-1995` (draft PR #3). **Definition of status:** authored and committed browser/Unreal **source**, plus a simulated JavaScript test; **Unreal 5.7 compilation, actual WebGL GLB visuals and 720p captures remain unverified**. Under Doctrine 000, a source implementation is not production-ready.

## Player-experience defect addressed

A mission may have good scripted beats yet feel like a prototype when all the people remain idle, moving bodies show stationary feet, or the wrong character is drawn over the world. The browser's optional comparison renderer previously downloaded one generic external soldier and cloned it for Snake, Fox and enemy guards, obscuring the game's committed original character-specific models. Unreal's importer produced source clips, but the runtime fallback played only Idle when the finished Animation Blueprint did not exist. The binocular item also set FOV once, while the next camera Tick overrode it.

These are presentation defects with consequences for perceived identity, readability of anticipation/recovery, responsiveness and player trust.

## Implemented

### Browser: actual six-role skinned cast
- `browser/rigged_characters.js` now loads **local original source GLBs**, not an external generic soldier:
  `snake`, `fox`, `guard`, `shoot_gunner`, `machinegun_kid`, `pow_informant`. Loading the 3D review still requires the Three.js library via the configured CDN; default field rigs and gameplay remain local/offline.
- Each actor gets its own skeleton clone, role-appropriate GLB, source animation clips, independently owned mixer, orientation matching the game's +X forward axis, and depth occlusion from active real solids. Imported model and animation validity is checked **before** hiding the local fallback. A missing role fails closed.
- `browser/animation_intent.js` resolves an action using the actual game-owned posture/evidence/interaction state: down > hit > boss shot/reload/telegraph > melee/knock > reach > captive/radio > aim/cover > crouch/prone > velocity-derived locomotion > alert/search/idle. Source game state owns combat; clips never reveal unseen enemies, move geometry or change mission flags.
- Idle performance has deterministic, cast-offset variations; real displacement controls walk/run pace. Cosmetic dialogue-driven jaw and brow accents are approximate, **not** facial performance capture or phoneme lip sync. Mixers pause with game time.
- `browser/game.js` sends a **single copy** of Snake, Fox, live ordinary guards, the first-floor informant and the individual bosses, including actual Shoot Gunner combat phase. The HTML options wording now accurately describes the comparison.
- `browser/tests/test_animation_intent.cjs` checks selection precedence and named clip contracts; campaign regression verifies cast identities, persistent scene-state transitions and no duplicated boss.

### Unreal: visibly alive even without production Animation Blueprints
- Added `Public/MGPrototypeAnimationFallback.h`: play imported, stable `AS_<Role>_<Clip>` AnimationSequences, changing on state transitions rather than requesting assets every frame. **An existing valid AnimationBlueprint takes precedence.** Clips are cosmetic and never write mission flags, awareness or damage.
- Snake: actual movement speed, sprint, crouch/prone, stance-specific aim, cover, melee/knock, hit, reach/body drag, radio and varied idle. Disarmed costume swaps reset the selected sequence (but the separate disarmed model asset still requires production).
- Guards: actual patrol velocity, searching/caution, evidence-driven aiming and reloading, hit reaction, knockdown, and repeatable restore of fallen bodies without double rotation.
- Gray Fox: source captive performance until **real** rescue state switches to wary idle. No duplicated Fox actor.
- Shoot Gunner: shot/shoulder/reload/recovery and down performances triggered by actual boss state.
- Machinegun Kid: its **independent boss Tick** now runs a distinct rig sequence, rather than silently skipping Guard animation updates.
- Added right-hand fallback attachment: where an artist-authored `weapon_r` socket is absent from an otherwise valid imported GLB, Snake, guards, Shoot Gunner and Machinegun Kid attach their weapon visuals to the actual `hand_R` bone. This is a **presentation fix**, not an approval of handgrip orientation or shot-source position; align weapon pivots, muzzle/bore and casing ejection in Unreal PIE before shipping.
- Fixed the binocular scope so `TargetFOV` is actually governed by scope state every Tick. A newly selected non-binocular item exits the scope without a permanent camera override.

## Executed/validated in this authoring session

- JavaScript source syntax for changed game, tests, source animation intent and GLB view module (module imports stripped for the syntax-only check): **passed**.
- `test_animation_intent.cjs` and `test_campaign.cjs` executed through a V8 compatibility harness with stubbed Node filesystem and DOM/WebGL: **both passed**. Campaign logged **17 PASS checkpoints**, animation-intent test logged one additional PASS. This verifies source mechanics/state transitions, not a rendered viewport.
- Manual cross-engine source contract check: **13/13 assertions passed** for loader/clip paths, actor identity, camera FOV, fallback calls and authored-ABP precedence.
- The added Python regression file and local PowerShell preflight/packaging calls are committed. Windows packaging now refuses to claim a functional character build without five native active skeletal meshes **and sixteen actual native gameplay AnimationSequences**, including Machinegun Kid's distinct model. These guards establish necessary inputs; they are not proof that clips were successfully imported or played. **Native Node, full Python unittest discovery, Rust cargo tests, real WebGL/GLB load, GLB appearance review, Unreal 5.7 C++ build, native imported .uassets, PIE, headset/TV/handheld UX and real user testing were NOT executed here.**

## Known visual/implementation limitations

The existing 22 source GLBs are original stylised 18-joint procedurally authored prototype rigs, not MGS4-quality production PBR assets. They lack individually crafted realistic heads/skin/hair/clothing, facial topology, phoneme rigs, motion capture, animation retargeting, hand-weapon IK and proper look-at. The six-role browser GLB review is not the full 22-character roster on screen: other characters await their canonical appearances. Unreal uses single-node fallback switches **without pose blending**, locomotion/aim layering or motion matching; professional AnimBPs and rig-conversion art remain necessary. Visual timing, roots, floor contact and weapon attachment must be verified from real screenshots.

## Next acceptance pass for tester

1. Pull this branch, run `scripts/unreal_tester_preflight.ps1`, import source characters with `Content/Python/import_field_character_glbs.py`, then compile Unreal 5.7 and open PIE. Verify all 22 imported source assets, with five active named actor rig types (Snake, Fox, guards, Shoot Gunner and Machinegun Kid); the POW informant is a role-specific GLB in browser review but still an Unreal gameplay proxy. Confirm actual imported clips and correct units. Report import warnings rather than silently accepting static meshes.
2. Record Snake locomotion-to-aim, crouch-to-prone, cover/movement/drag, weapon grip on imported `hand_R`, and captive-to-rescued Gray Fox; confirm no T-pose and that the motion fits the actual capsule/collisions.
3. Film idle guards, search/warning/reload, CQC KO, dragged body and checkpoint re-entry; test Shoot Gunner's three-shot shoulder/shot/recovery and Kid's independent burst/reload.
4. Test a 10-second binocular view while stationary, walking, crouching and returning to pistol; scope FOV must not revert automatically, with no omniscient guard outlines.
5. Play the browser on localhost with review mode disabled and enabled; verify **all six** identities render once, real wall occlusion, actual GLB load, compatible controller/keyboard input, frame pacing and no loss of original local offline play when the Three.js CDN is unavailable.
6. Promote animation to `running_in_engine` only on compiled/editor evidence, and to `production_ready` only after quality-approved character-specific art, AnimBP blend/IK, audio, accessibility and novice/veteran playtest evidence. Do not confuse attractive still captures with functional gameplay.

**Source authority:** original 1987 MSX2 progression remains canonical. These visual improvements do not alter original story causality, card gates or AI knowledge.
