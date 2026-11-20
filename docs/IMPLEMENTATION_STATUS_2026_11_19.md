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


## 19 November 2026 — quality and full-slice pacing implementation

**Committed source (not compiled in UE):**
- The actual 1987-style **Big Boss call now displays successive lines every 1.65 seconds after the incoming connection**, instead of dumping a long transcript; still movable, instantly closable and tuneable. Only the initial call is staged. The full objective/intelligence survives a skip.
- Rust `pacing.rs` and the Unreal mission fallback now share **12 distinct values** across prelude/clue (0), detention (9), cell (1), breakthrough (2), revelation (3), escape (4), approach (6), unarmed recovery (7), armed boss (8), defeated-boss search (10), exit/card (11) and complete (5). Before, defeating the boss jumped straight to the terminal quiet phase and suppressed all post-boss assistance.
- HUD objectives now describe meaningful *goals*, avoiding premature revelation of the exact weak wall, door, transmitter and access-card method. Optional hints are staged **28/75 seconds** after stalled progress and are delayed by real exploration; genuine guard alarm or Shoot Gunner firing/aiming suppresses them. Speed-solvers never wait for a hint.
- Physical level correction: prelude crate/utility cover now extends above guard eye trace, guards no longer stage near the obstructing booth edge, the truck and custody marker sit in a wider ~360cm manoeuvre lane, and corridor guards get appropriately high cover too. Player learns POW clue and can choose how to be seen before detention.
- Custody guard code requires an actual unobstructed LOS **and approximately 0.75 seconds of challenge procedure** before successful compliance; threatening approach temporarily delays a compliant arrest. Blueprint acting event `OnCustodyChallenge` is available but no final guard VO/animation is present.
- Guard perception debug shapes are off by default in PIE, and can be enabled per guard for diagnosis.
- `AMGGrayFoxMission::OnPacingBeatChanged` now signals the committed phase and suggested dramatic intensity **once** per transition, including save restore. This is a pure presentation notification for eventually authored music, lighting and Sequencer and **cannot be used as mission-state authority**.
- [data/beat_contracts.json](../data/beat_contracts.json) now documents 18 beats and [data/area_contracts.json](../data/area_contracts.json) has nine source blockout areas, including POW, custody, prison, Shoot Gunner equipment recovery, combat and Card 3 aftermath. Old descriptions of immediate E-to-break walls and first-load cell spawn were corrected. 
- Added [tools/test_prelude_layout_and_custody.py](../tools/test_prelude_layout_and_custody.py) and [tools/test_pacing_integration_source.py](../tools/test_pacing_integration_source.py) as local dependency-free source checks.

**Quality evidence and unmet gates:** The connected GitHub source was inspected and source contract topology/required fields checked. These are not actual engine builds or measured gameplay. No UE5.7 compile/PIE packaged run, reliable animation/voice, authored level lighting, original adaptive music stems, real Sequencer scenes, final character rigs, profiler capture, or novice playtest has been completed. The source blockout and technical actor behaviour are not the promised final game art. Original MSX2-to-J2ME geometry/event equivalence is still unverified.

**First-time game-design acceptance run when Unreal is available:** Start, hear/tune/skip radio; traverse either yard path; meet POW; use real guard challenge without being killed or appearing complicit; check captured-cell outfit/inventory; strike first wall three times; find Fox; strike second wall four times; use guard distraction/cover; traverse physically into boss wing; survive three shotgun shots by observation; recover bag, discard transmitter; fight; explore Card-2 room, claim Card 3 and leave without losing checkpoint state. Repeat with early/late clue discovery, controller, muted audio, low FPS, death near each transition and manually skipped dialogue. A good curve isn't proven until unfamiliar players can complete those routes.


## 19 November — layered guard state machine and evidence-responsive world

Guard source now distinguishes (1) **assigned duty** Patrol/Prison/Custody, (2) eight named local awareness phases, (3) eight evidence-source types, (4) nine tactical/acting postures, and (5) five independent installation security conditions. Source includes state-change Blueprint events, damage that requires a visible and cancellable melee windup, collision-aware movement, physical body discovery, actual wall noise, radio sightings with a frozen direct-sight payload, search and return to correct patrol axis. A real transmitted contact raises facility Alert, body evidence raises Caution; security transitions via Evasion and StandDown instead of resetting in one frame. Facility Alert changes attention and temporarily disables routine custodial surrender but never gives through-wall knowledge. The opening radar now uses its own yard projection.

Added temporary nearby guard subtitles for actually observed challenge/notice/search/contact/report/body events; final professional voice and animated acting remains *absent*. See [guard state/feel bible](GUARD_SECURITY_STATE_AND_FEEL_BIBLE.md) and [machine state contract](../data/guard_ai_behavior_contract.json). [Source regression checks](../tools/test_guard_state_layers.py) are committed but **NOT EXECUTED** in this GitHub-only pass; no UE5.7 compile, AI simulation runs or packaged playtesting is claimed. Difficulty values, state timers, and group communication boundaries require unfamiliar-player tuning before release.

## 20 November — sensor evidence fairness patch (committed source, not UE-validated)

On draft PR #3 the Rust-backed and C++ fallback guard paths no longer save Snake's exact current location every frame from hearing alone. Footsteps now provide an *uncertain, quantised* audible sector at a bounded refresh rate (coarser/less frequent through cover). Fresh direct sightings, body discoveries, colleague reports and wall impacts take priority. Only first-hand visual confirmation renews the visual-contact timer; audible movement by itself cannot sustain confirmed pursuit indefinitely. Guard injury interrupts an unfinished radio report, with bounded eligibility to retry after new visual proof.

A meaningful sound suspicion now causes a **measured walk** toward the imprecise sector for a limited interval, rather than remaining stationary or charging at full confirmed-contact speed. The 185 cm/s pursuit gait is reserved for a recent firsthand sighting; an evidence-only investigation uses 115 cm/s. These thresholds and times are provisional hypotheses, not original-MSX2 numeric facts.

Structural source assertions and machine-contract updates are committed in `tools/test_guard_state_layers.py` and `data/guard_ai_behavior_contract.json`. No UE5.7 compile, headset listening study, acoustics simulation, final character animation, hardware/controller test, or user playtest was performed. Prior art/VO/NavMesh gaps remain.

## 20 November — young-Snake hair, animation crossfades and human VO source routing

**Committed to the existing PR #3 draft branch:**
- An original, editable 1995 brown hairstyle OBJ/MTL, non-overwriting Unreal Editor importer and actual C++ head-bone attachment. The attachment is reapplied after disarmed/infiltration mesh swapping; missing rig/stock head socket hides the object rather than producing a floating haircut. **Final hero mesh/face, skin weights, groom and hair dynamics do not exist in this source branch.**
- Shared `UMGCharacterAnimInstance` performance-action arbitration and transition events for Snake, guards, prisoner Fox and Shoot Gunner; expose interruption and elapsed-duration crossfade hints, and keep transceiver listening additive to locomotion. **No authored AnimBP graphs/montages or tested IK are shipped.**
- Optional per-line VO clip loading and lifetime-aware transceiver speech. The opening line reveal now follows the duration of real clips if imported; manual contact reveals the whole transcript. Retuning or closing immediately stops pending VO. Four display-only caption wraps are grouped as continuous spoken sentences.
- Diegetic optional Gray Fox rescue, Shoot Gunner introduction and eight physical guard callouts, with matching subtitles already shown through the mission or guard system. The default remains audio-free and accessible.
- Six speaker profiles and a 45-clip production manifest; offline ffmpeg two-pass loudness matching script and non-destructive Editor importer. All clips are **pending actual voice actor recordings**; no false voice synthesis, actor cloning or imported SoundWaves.
- Asset inventory now records new source items without claiming approval; animation/voice/hair quality gates remain open.

**Verification boundary:** source contract/manifest and mesh topology can be structurally inspected; only real Unreal engine import, compiler and novice level playtesting can validate actual smoothness, hand-head integration, performance, character likeness and final dialogue quality. Keep PR in draft.


## 20 November — narrative negative-space doctrine, factual edition correction, five optional world traces

Narrative expansion now follows [Doctrine 006: Narrative Negative Space](DOCTRINE_006_NARRATIVE_NEGATIVE_SPACE.md). The rule is to explain how the 1995 fortress functions without spoiling who actually commands Outer Heaven, why real Big Boss's guidance becomes misleading, Gray Fox's limited information, or the retrospective identity meaning of the `The Phantom Pain` ending. The original MSX2 is the primary source; earlier references to `metalgear.dr-sunny.info` as an original MSX script were incorrect because that site identifies its transcript as the NES version. Primary MSX2 script and Japanese MSX2 comparison are now explicitly referenced.

**Actual C++ source implemented in `MGGrayFoxMission`:** optional cargo, duty/transport, cell repair, inventory-custody and dog-kennel evidence actors. All five are physically placed against existing scene geometry, require the player's facing/range and unobstructed world-space interaction test, and use the already implemented accessible subtitle channel. None calls `SaveProgress`, `ApplyAction` or any security/plot mutation. This is environmentally grounded optional flavour; no main objective needs them.

**Production status:** replace greybox cube-prop markers with approved physical paper/clipboard/supply art; review in engine for legibility at 720p, prop placement, camera obstruction, audio availability, stealth-risk interference and source edition fidelity. Four new Python source tests were authored and 11 connector-side checks passed; Python tests, UE compilation and player tests remain **not executed**. Keep PR #3 a draft until actually integrated and validated.
