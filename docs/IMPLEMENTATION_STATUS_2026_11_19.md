# OUTER HEAVEN — IMPLEMENTATION / EVIDENCE CHECKPOINT
**19 November 2026. Draft PR #3, branch feature/character-art-pipeline-1995.**

This checkpoint is a **truthful development state** for resuming work, not a marketing milestone. Governing doctrine: [How We Build](DOCTRINE_000_HOW_WE_BUILD.md). User goal: lore-accurate, truly playable and cinematically ambitious original *Metal Gear* (1995-setting) remake; source reference is original 1987 MSX2, product source target remains the supplied Connect2Media J2ME v3.3.0 edition.

## Implemented in committed source

- Separate character art bridge in Unreal player/guards/Gray Fox, shared animation-state parameters, visible weapon-socket prop logic, movement/pitch/yaw signals and capsule-driven prototyping from prior commits on the branch. **Assets and full AnimBPs not present.**
- Source-authoritative Rust Gray Fox mission transaction set extended through: optionally learnt POW detention clue, capture and loss of gear, two walls, Gray Fox rescue, prison exit, Shoot Gunner encounter, gear recovery with transmitter planted, optional transmitter removal, boss defeat, **Card 3** pickup and slice completion. Legacy five-step sequence/checkpoints remain supported.
- Unreal mission fallback and save validation now recognise extended flags and support old schema-v1 checkpoints as well as schema-v2 new flags; the gear-confiscated visual state derives from actual recovery (not merely historic seizure). Card 3 is modelled as an inventory item and reconstructed after a loaded pickup flag. **Not an actual completed Shotmaker arena or earned opening.**
- Original TX-55 sixteen-charge ordered-leg sabotage: dedicated Rust simulation and backward-compatible v1 C ABI extension, with written tests rejecting incorrect leg, duplicate montage notify, no charge, corrupted state and post-destruction resets. Source contract retains Madnar's **first 15 only**. **Not integrated into Unreal TX-55 arena or any final model.**
- Full 1995 continuity treatment: real Big Boss's radio presence vs hidden Venom final opponent, canon-scope/retcon boundaries, Snake's interrogative echo rhythm, earned capture, finite radio knowledge, Gray Fox, Schneider, Madnar/Ellen, TX-55 and original edition naming differences.
- 20 individual named/encounter role working character bibles and machine-readable completeness registry; additional full protagonist Snake bible and dedicated TX-55 boss bible.
- Seven detailed cinema contracts (capture, Gray Fox, Shoot Gunner intro and aftermath, Madnar, TX-55, Venom/Big Boss finale), shot intentions, player knowledge, durable gameplay event ownership, skip parity, assets and failure modes. **No actual final Sequencer scene exists**.
- Asset/motion candidate quality gate, no false acquisition claims, legally checkable permission/source manifests, source blending standards; import and 720p evidence outstanding.
- Python validators and unittest source-regression cases added for cast status, TX-55 source contract, cinematic claims, asset approvals and extended mission state parity.

## Source inspection performed, and limits

Confirmed through connected GitHub: files committed to the branch; 20 registry entries have existing bible files; all seven scenes contain shot lists; TX-55 contract has sixteen directions and marks fifteen known; necessary Rust headers/modules and Python validators exist; mission flags include Card 3 and transmitter. **This is structural source verification**, not proof Python/Rust tests ran or UE code built.

**Not executed on an Unreal-capable host:** cargo test/build, Python full test suite, actual UE5.7 C++ compile, Blueprint/AnimBP construction, real rigs or animation retargeting, face/voice/cutscene rendering, integrated HUD and audio, playable new area, packaged build, controller/720p hardware tests, unfamiliar-player research.

## Known incomplete or potentially inconsistent systems

1. **The playable Unreal prototype still starts already captured in the cell.** POW clue and nonlethal seizure are designed and encoded as state events, but neither authentic pre-capture encounter nor cinematic exists.
2. **No Shoot Gunner actor, recovery-room geometry, transmitter inspection/removal interface, boss ballistic behavior, Card 3 physical pickup or escape chapter-ending interaction has been integrated in-world.** Unreal can represent the new state but cannot earn all transitions through play.
3. **Equipment inventory still contains provisional STUN pistol/ration,** not a source-authenticated complete pre-capture inventory. Card 3 recovery is flag-based. Exact consumable/ammunition persistence across all checkpoints is NOT solved.
4. **C++ source remains uncompiled**; audit API signatures and constructor/lifecycle logic on UE5.7 before merging. The player art bridge needs real mesh/AnimBP assets and hand/muzzle socket verification.
5. **TX-55 has no built arena, rig, cinematics or gameplay bridge.** The stand-alone Rust transaction is merely an engineered core for later integration.
6. Source verification is **original MSX2 comparative**. Do not assert identical J2ME event order, geometry or dialogue until archive-specific extraction and cross-check.
7. Voice acting, licensed music, visual character quality and active Sequencer skip/input restoration require independent proof, not a generated storyboard.
8. PR #3 is intentionally **draft**. Do not merge as an art/visual/gameplay completion claim.

## Next vertical-slice critical path (correct quality order)

1. Run local Python unittest and Rust tests, fix any failures; compile Unreal 5.7 with installed toolchain and examine warnings.
2. Directly audit the supplied J2ME edition for story transitions and compare source MSX2 events. Keep evidence ledger and localisation aliases.
3. Build pre-capture playable infiltration and bounded guard custody: POW intelligence, tactical context, believable nonlethal arrest. Make scene ownership and skip-safe capture a discrete transaction.
4. Build realistic cell wall stages and physical Gray Fox encounter; 1995 clothes and character/animation/Codec continuity.
5. Build Shoot Gunner encounter, owned-equipment pickup, discoverable transmitter and genuinely collected Card 3 with a checkpoint-safe inventory snapshot.
6. Author real in-engine Sequencer shots, working Skip and handback, voiced/captioned dialogue, camera and performance polish. Earn the cinematic runtime through substantive gameplay.
7. Acquire/create lawful recognisable **fully rigged and animated** Snake, human Fox, distinct guards and Shoot Gunner; motion-fusion and PBR QA in actual 720p game view. Mix animation libraries only after contact/retarget evidence.
8. Build subsequent full-game levels and TX-55 rig/hangar, fifteen-step Madnar log, deterministic sabotage bridge and multi-stage destruction; close with fair Venom confrontation that preserves the 1995 Big Boss revelation.
9. Packaged novice playtesting before advancing source contracts to engine-tested or shipped.

**Quality principle:** never equate a design bible, toolchain source, placeholder model, unexecuted test, or imported asset with the player's experience. Keep the right story **and** tell it through credible choice, cinematic timing, world response and physically coherent art.


## 19 November 2026 — Shoot Gunner connected-wing implementation pass

**Added to the draft branch since this snapshot:** `MGShootGunnerActor.h/.cpp` source actor with a three-shot wide spread volley, constrained sight/muzzle raycasts, cover and explicit warning/recovery phases. **Current timings are tuneable 3D interpretations, not measurements of the 1987 attack.** Player handgun firing is separated from the original nonlethal test weapon.

The Unreal prison greybox physically connects east into a boss wing; side rooms include a repeatedly punched recovery door, the confiscated bag, a **planted transmitter inside the recovered item inventory**, a Card-2 storeroom containing Card 3 and a chapter gate. Events advance through the existing Rust/C++ flag contract, and SchemaVersion 3 save logic tracks two weapon ammunition pools and migrates old prison checkpoints. The original transmitter gives the boss approximate periodic positional hints but cannot confer x-ray bullet hits; the player can discard it through the item menu. **This is an encounter implementation at source level, not a proven playable Unreal level.**

**Original editable source assets authored:** `assets/source/shotmaker/riot_gun_proxy.obj` (19 separately editable parts), `riot_gun_game_mesh.obj` (single mesh with groups), matching MTL; original 22,050 Hz mono WAV cue files `Shotmaker_AimCue.wav`, `Shotmaker_Blast.wav`, `Shotmaker_Cycle.wav`. An Unreal Editor Python script imports these exact source assets into the paths expected by the C++ actor. No editor-side asset import was run; none of these is a final MGS4-quality licensed animation/rig. A procedural muzzle/reload cue is not professional gun Foley.

**QA fixes:** cover collision raised to actual projectile height, corridor opening widened, boss radar projection corrected, periodic tracker hints rather than every-frame omniscience, and source tests adjusted from V2 to V3 save schema. New source/binary smoke checks verify key constraints and WAV headers once run locally.

**Still blocking production quality:** authentic original version geometry/dialogue verification, earned pre-capture gameplay, specific asset licensing + full-body skeletal mesh rigging/retarget and animation, 3D actor–weapon hand/shoulder/muzzle socket contact, actual UE Sequencer cinematic playback, facial/VO/audio mix, UE5.7 compile and packaged novice playtesting. Never label this 'finished' until these gates are met.
