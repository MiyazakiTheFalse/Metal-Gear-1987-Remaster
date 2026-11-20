# Metal Gear (1987) Remaster

PC-first, Switch-targeted modern 3D remake of the supplied Connect2Media **J2ME Metal Gear v3.3.0** edition. The direction is free-camera MGS4/MGSV-style movement, stealth and aiming, while faithfully reconstructing the original game's locations and mission progression.

> **Current status (20 November 2026): early-campaign source prototype, NOT a completed game.** The browser has an executable-by-source Gray Fox-to-west-lift sequence with provisional models; there is no verified downloadable browser package in this session. The UE5 project has **not** been compiled or packaged in this environment. No playable Windows .exe or Switch build is established. The browser and Unreal source now extend from Gray Fox through Shotmaker, Machinegun Kid, Building 1 roof/Hind D and the Building 2 tank road. These later encounters are provisional, not Unreal-compiled or production-ready; the remainder of the original campaign still needs implementation. A UE-capable build host is needed to produce PC playtest packages.

## Unreal tester checkout — current development branch

For the latest **uncompiled** Unreal 5.7 C++ source and committed skinned GLB assets, check out `feature/character-art-pipeline-1995`, tracked in [draft PR #3](https://github.com/MiyazakiTheFalse/Metal-Gear-1987-Remaster/pull/3). Do **not** assume `main` includes this pass.

**Tester instructions:** [Unreal Tester Handoff](docs/UNREAL_TESTER_HANDOFF_2026_11_20.md). Run `scripts/unreal_tester_preflight.ps1` on a Windows UE5.7 build host, import the character GLBs through `unreal/MGRemaster/Content/Python/import_field_character_glbs.py`, then run `scripts/build_windows.ps1` for a locally generated Windows Development package. None of these steps is a verified Unreal build in this authoring environment. The four skinned characters are original, procedural prototypes, not photorealistic finished art.

## Title screen and metagame (browser source, 20 Nov 2026)

The main browser title now uses independently authored MGS-inspired tactical artwork, responsive menu layout and reduced-motion handling. New Game, Continue (real checkpoint-gated), Mission Briefing, VR Shoot Gunner rehearsal and Options have working keyboard/controller routes. Selecting New Game with an existing checkpoint requires an explicit Cancel-default confirmation. The briefing uses original 1987-informed illustrated Snake and Big Boss portraits. The radio portraits are unique source SVGs for eight 1995 identities, currently illustrated rather than realistic facial scans.

Development status and remaining native Unreal front-end/audio/UX tasks: [Metagame Title and Front-end Contract](docs/METAGAME_TITLE_AND_FRONTEND_1995.md). This is **not** a verified in-browser visual/UX playtest, completed Unreal intro movie, final VO, or production character render.

## Implemented in source

- **Unreal Engine 5.7** C++ prototype chamber with basic geometry, cover, two guard actors and a HUD.
- Free third-person camera (mouse/right stick), camera-relative locomotion, sprint and crouch.
- Smooth over-shoulder aiming, aim strafing, shoulder swapping and prototype nonlethal stun shot.
- Guard patrol and awareness using **Rust** simulation over a native C ABI (sight, hearing, suspicion, alert).
- Original J2ME archive audit/extraction tool and synthetic tests; original content stays local.
- Four optional graphics presets, with Performance settings on first launch.

These features are unverified source implementation, not finished gameplay or production art.

## Repository

- `unreal/MGRemaster/` — UE 5.7 project, C++ character/camera, guard, game mode, HUD, video/input configuration.
- `rust/` — independent native perception, record reader and C ABI.
- `tools/` — licensed source archive inspection + local extraction, tests.
- `scripts/build_windows.ps1` — packaging entrypoint on a Windows UE5.7 build host.
- `docs/` — source audit, gameplay scope, graphics policy and build notes.
- **1995 Solid Snake protagonist bible:** [binding identity/rig/aim/animation design](docs/SOLID_SNAKE_1995_PROTAGONIST_BIBLE.md), with [machine-checkable contract](data/solid_snake_character_contract.json). This is an authored specification, not imported/verified final character art.

## Additional full-game lore and character bibles

- [1995 continuity and cinematic treatment](docs/OUTER_HEAVEN_1995_CONTINUITY_TREATMENT.md) preserves the 1987 player perspective while reconciling later Big Boss and Venom canon.
- [TX-55 boss bible](docs/TX55_METAL_GEAR_BOSS_BIBLE.md) preserves the 16-leg-charge sabotage puzzle and plans its cinematic/engineering climax.
- [Significant-character individual bibles](docs/CHARACTER_CAST_BIBLES/README.md) with [auditable registry](data/character_bible_registry.json) cover original cast, encounters, allies, guards and both Big Boss identities. These are working source documents **not** imported models, rendered cinema or completed full-game systems.

The [in-engine cinematic direction bible](docs/CINEMATIC_DIRECTION_AND_CUTSCENE_BIBLE.md) and [shot/skip contracts](data/cinematic_scene_contracts.json) cover capture, Gray Fox, Shoot Gunner, Madnar, TX-55 and the final confrontation. These are source-level cinematic designs, not finished Unreal sequences.

## Global production doctrine — binding

**Doctrine 000 — [How We Build](docs/DOCTRINE_000_HOW_WE_BUILD.md)** governs every other doctrine. Product quality is defined by the *method*: research and provenance, player-first intent, integrated execution, actual evidence, accessibility, source fidelity, performance and maintainability. Its [Doctrine 005 — Spatial Structure](docs/DOCTRINE_005_SPATIAL_STRUCTURE.md) companion binds topology and room rhythm to the same standards, with [area contracts](data/area_contracts.json). When requirements conflict, record an explicit decision and test the player's experience instead of allowing undocumented shortcuts.

Every player-facing beat must be designed as **one synchronized audiovisual, interactive and temporal experience**, not mechanics first and cosmetic polish later. See [Doctrine 004](docs/DOCTRINE_004_INTEGRATED_EXPERIENCE.md) and the [beat registry](data/beat_contracts.json). Visual foreshadowing, spatial sound, inputs, timing, AI knowledge, durable state, interruptions and accessibility must agree; a polished asset is not proof of a finished beat. The registry begins with nine source/design-stage contracts and is not exhaustive. Local regression checks are required before packaging; Unreal playtesting is still outstanding.

## Local checks

```sh
python -m unittest discover -s tools -p 'test_*.py'
python tools/validate_beat_contracts.py
python tools/validate_area_contracts.py
python tools/validate_solid_snake_contract.py
python tools/validate_full_game_bibles.py
python tools/validate_cinematic_scenes.py
python tools/validate_asset_selection.py
# New source-only Shotmaker artifact and encounter checks:
python -m unittest discover -s tools -p 'test_*.py'
python tools/inspect_and_extract.py /path/to/Metal-Gear_J2ME_EN_v330.zip --output content
cd rust && cargo test && cargo build --release  # Includes engine-neutral TX-55 sabotage tests
```

## Shotmaker source pass (19 November 2026)

The [Shotmaker encounter contract](data/shotmaker_attack_contract.json) now distinguishes original three-shot shotgun spread from adjustable modern 3D cadence/spread parameters. The Unreal actor and connected prison-to-boss wing encode physical recovery-room breach, reclaimed handgun, inventory transmitter with radio beacon effect, Card 2 → Card 3 progression, and a fair line-of-sight attack system. [Original editable riot-gun geometry and WAV cues](assets/source/shotmaker/README.md) include an [Editor import script](unreal/MGRemaster/Content/Python/import_shotmaker_source_assets.py); this is **source only**—no UE5.7 compile, imported UAssets, finished skeletal models, animation or complete cinematic. See [source/evidence checkpoint](docs/IMPLEMENTATION_STATUS_2026_11_19.md).

## Vertical-slice audio and art completion audit (19 November 2026)

[Source asset inventory](data/vertical_slice_asset_inventory.json), [readable audit](docs/VERTICAL_SLICE_ASSET_COMPLETENESS_AUDIT.md), and [original transceiver WAV cues](assets/source/transceiver/audio/README.md) separate existing source files from *actual* Unreal imports. The new in-game call sound hooks **require importing** those SoundWaves using [Editor script](unreal/MGRemaster/Content/Python/import_transceiver_audio.py). No final voice, music stems, skinned character assets, or real Sequencer cutscenes are yet present. Validate source inventory with `python tools/validate_slice_asset_inventory.py`.

## Windows packaging

On a Windows machine with UE5.7, Rust MSVC, Visual Studio C++:

```powershell
.\scripts\build_windows.ps1 -UnrealRoot 'C:\Program Files\Epic Games\UE_5.7'
```

The resulting `dist/Windows/` package must be smoke-tested before distribution. No UE-capable runner is currently connected; **no executable has been generated**.

The prison source prototype now includes a non-pausing, optional 1995-style transceiver with architectural dialogue from Kyle Schneider (120.79) and separate Big Boss guidance (120.85); C/Start opens it even when weapons are confiscated. The dialogue is remake-authored and currently displayed as text, not recorded VO. The early mission objective no longer gives away the wall. The prison's destructible mission walls intentionally **do not** trigger automatic cover: E/X breach interaction retains priority. Knocking is a sound-location distraction, not a direct player-position reveal. See [wall/knock implementation contract](docs/WALL_KNOCK_AND_BREACH_CONTRACT.md). No UE5.7 build validation or finished skeletal animations are claimed.

## Prototype controls

| Action | Keyboard/mouse | Gamepad |
|---|---|---|
| Move | WASD | Left stick |
| Camera | Mouse | Right stick |
| Aim | Hold RMB | Hold LT |
| Stun | LMB while aiming | RT while aiming |
| Sprint | Shift | Press left stick |
| Crouch | Ctrl | B/Circle |
| Wall cover | Move into full-height wall; V manual | Move into full-height wall |
| Knock (in cover) | F | A / face-bottom |
| Corner peek | Hold aim + slide towards wall end | Hold LT + move towards wall end |
| Breach mission wall | E (in reach, face wall) | X / face-left |
| Swap shoulder | Q (outside radio) | Press right stick (outside radio) |
| Optional transceiver | C to open/close; Q to tune | Start/left special to open/close; press right stick to tune |
| Graphics quality | F6 | Options menu pending |

## Event architecture — Doctrine 002

The [event doctrine](docs/DOCTRINE_002_EVENTS.md) governs how every rescue,
radio call, security trigger, hazard, objective and cinematic is authored.
The standalone [Rust event runtime](rust/src/mission_events.rs) now models
stable trigger IDs, atomic mission flags, once-only activations, presentation
cues and versioned mission snapshots. Five Rust unit tests are written,
but neither Rust nor Unreal has been compiled in this authoring environment.

The [event evidence ledger](data/event_ledger.json) records six event-family
candidates with sources and interruption rules. All are still marked
COMPARATIVE_PENDING_J2ME: original MSX2 documentation is not proof that
the supplied Connect2Media J2ME game shares the same exact event triggers.
Run `python tools/validate_event_ledger.py` and the associated unit tests
before promoting a candidate to verified. Event simulation is not yet
connected to Unreal Gameplay Tags, save games, AI StateTree or sequencers.

## Next checkpoints

1. Complete a real UE5 Windows compile/package test and fix any engine errors.
2. Build an install-free low-spec PC test package and distribute through GitHub Releases.
3. Upgrade animations, physical traversal, cover and CQC before claiming MGS4/MGSV-quality feel.
4. Decode J2ME level/events and rebuild the first authentic mission area.
5. Introduce UI, saves, audio, advanced guard tactics, accessibility and console performance plans.

Switch requires official Nintendo SDK access, Unreal console integration and devkit testing. GitHub Actions paid CI has not been configured.

## Asset proportions and source-of-truth ledger

The first six independently authored greybox OBJ modules live in assets/greybox,
with scale specifications in assets/asset_dimensions.json and original/later-game
source claims recorded in data/reference_ledger.json.

Run the dimension/provenance checks before every asset import:

    python tools/validate_asset_dimensions.py
    python -m unittest discover -s tools -p 'test_*.py'

On an Unreal-equipped workstation, enable the PythonScriptPlugin and run
unreal/MGRemaster/Python/import_greybox_assets.py from Unreal's Python console.
It imports the source OBJ models and rejects bounds that differ from the ledger by more
than 1 cm. Imported meshes do not constitute a completed level or a playable game.

See docs/ASSET_REFERENCE_AND_SCALING.md. The J2ME 30x24 tile hypothesis is
not a confirmed real-world room measurement; no fabricated metres-per-tile is used.

## Gray Fox prison rescue — source vertical slice

The first rescue scene has been authored as an Unreal 3D **greybox** and is now
the default generated mission scene. Its provisional geometry has two weakened
walls, a temporary Gray Fox proxy, a guarded escape lane, and contextual mission HUD.
Use E or the gamepad left-face button near a target to interact. F9 resets the
mission-specific save slot and reopens the current level. Capture confiscates the
prototype stun weapon; mission flags and checkpoint positions persist.

Rust now holds the ordered Gray Fox transition rules and a non-blocking pacing
model (experimental 28/75-second optional hint candidates, exploration-aware suppression, quiet narrative beat, escalating escape).
The Unreal scene has a reduced fallback state machine for iteration without Rust.
See [pacing doctrine](docs/DOCTRINE_003_PACING.md) and
[rescue implementation notes](docs/GRAY_FOX_RESCUE_IMPLEMENTATION.md).

**Still missing:** verified J2ME prison map and script, final Gray Fox/Snake
animations, real prison art, authentic scene audio, proper AI navigation/combat,
and compilation/playtesting on an Unreal-capable workstation. Source is not yet
a downloadable executable.
