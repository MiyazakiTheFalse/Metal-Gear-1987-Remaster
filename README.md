# Metal Gear (1987) Remaster

PC-first, Switch-targeted modern 3D remake of the supplied Connect2Media **J2ME Metal Gear v3.3.0** edition. The direction is free-camera MGS4/MGSV-style movement, stealth and aiming, while faithfully reconstructing the original game's locations and mission progression.

> **Current status: source-only vertical slice.** The UE5 project has **not** been compiled or packaged in the available environment. There is no playable Windows .exe or Switch build yet. A UE-capable build host is needed to produce PC playtest packages; testers will not need the editor.

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

## First connected 1995 mission chapters — implementation source

The [Building One → Gray Fox prison → Building One runtime bridge](docs/1995_PLAYABLE_CAMPAIGN_BRIDGE_PASS.md) now gives the first two narrative chapters a persistent Unreal launch, player-triggered POW/capture route, actual prison gear recovery, character-relevant log and a post-rescue continuation checkpoint. Chapter and source-comparative story IDs persist via `MG1995CampaignV1`. The [chapter contract](data/1995_runtime_chapter_contract.json) and [Python regression assertions](tools/test_campaign_chapter_bridge.py) track this integration; [Rust chapter rules](rust/src/campaign.rs) establish future engine-neutral parity.

**Current truth:** Unreal and Rust source committed, **not compiled** or playtested; physical detention approach and missing building connections remain provisional until J2ME verification. Do not call this a complete playable game or a verified source level. A developer-only F9 reset from the Building One area clears test checkpoints and restarts at the beginning.

## Implementation pass — Missions 05–08 and browser playtest v2

**Unreal source:** Now has builders and interactions for proposed missions **1 through 8**. In particular the [Missions 5–8 implementation pass](docs/MISSIONS_05_TO_08_IMPLEMENTATION_PASS.md) adds the Madnar->Building Three exit, optional false Big Boss truck lead, guarded desert and oxygen passage, Coward Duck's **three physical hostage sightlines**, Card 8, an inner bunker witness/abort order, two security bypasses, the **16-step TX-55 demolition sequence**, a post-demolition self-destruct timer, final boss and left-ladder escape payoff. The former Card 7 collected-but-flag-not-recorded transition softlock is repaired. [Machine-readable mission registry](data/1995_numbered_missions.json) now clearly distinguishes *source world builder present* from *compile/playability verification*.

**Browser demo v2:** A separate Gray Fox playtest ZIP was built and tested locally (not committed as a binary). The browser now supports **timed left punch -> right punch -> kick**, contact-based guard damage, knockback, maintained adaptive audio, and functional **Game Over Continue Checkpoint / Exit to Title** callbacks. Its original gameplay, rig, music/audio and updated melee regression tests pass under Node. The browser demo **does not contain the later Unreal missions**.

**Acceptance remains open:** no Unreal 5.7 compile, cooked build, PIE playability or J2ME-exact spatial verification; no real Chromium WebGL/audio listening verification. Current rooms and bosses are a source-comparative prototype, not production-complete.

## Original Adaptive Audio — multi-engine integration pass

A **new original** sound bank now exists in the current conversation as separate downloadable bundles: 14 editable new MIDI compositions, 14 rendered music WAV loops, and 46 original WAV effects (**60 rendered WAV files**). The source game's 56 original J2ME MIDI files are **not** being redistributed. [Audio integration, cue IDs and instructions](docs/ORIGINAL_ADAPTIVE_AUDIO_IMPLEMENTATION.md) detail how to copy the same WAVs into Unreal and the browser.

**Unreal:** `MGAdaptiveAudioDirector` is spawned on title and all missions; it crossfades from actual mission/guard/boss/gameover state and routes movement, three-hit melee, choking, dragging, guns, rockets, mines, radio and menu effects through SoundWave assets. The Python editor importer checks SHA-256 before turning the WAV files into `/Game/Audio` assets. **Actual files are in the downloadable overlay ZIP, not Git-tracked binaries, and Unreal compilation/import/playback remains unverified.**

**Browser:** the expanded Gray Fox playtest ZIP includes WAVs, `audio_manager.js` for adaptive cues, positional effects, volume sliders and procedural fallback. The browser campaign and audio-bus source tests passed. The browser slice is not the eight-mission Unreal prototype; no full-game audio coverage should be claimed.

## Audio and failure-screen readiness

**Soundtrack status:** the source edition archive contains 56 original MIDIs (10 BGM, 4 intros, 42 sound effects); **there are no upgraded MIDI, WAV, OGG or other rendered music/sound files in this Unreal branch**. The separate expanded browser playtest has 19 procedural SFX cues and 3 ambient drone layers, but no audio file pack or adaptive MIDI score. See the [eight-mission audio inventory and coverage audit](data/1995_audio_coverage_audit.json). Sourcing, producing, mixing and importing appropriately licensed music/SFX remains unfinished.

The [Game Over / Continue / Exit-to-title implementation](docs/GAME_OVER_AUDIO_AND_MENU_CONTRACT.md) is added in Unreal source with a radio-loss overlay and preserved checkpoints, but its sound assets and finished animation are *not* present. Unreal compilation and audio playback tests remain outstanding.

## Three-strike Metal Gear Solid melee pass

The [MGS-style melee implementation contract](docs/THREE_STRIKE_MELEE_IMPLEMENTATION.md) is reflected in Unreal source: **left punch → right punch → right roundhouse kick**, with two rapid queued presses, physically timed hit windows, alternate guard flinches and a finishing knockdown. Current prototype shoulder/hip joints show separate moves; real skeletal animation clips can be assigned to the three exposed montage slots. No actual licensed attack montages or high-resolution character rigs are included yet. Source and timing regressions: `python -m unittest tools/test_three_strike_melee_contract.py` (not run as a UE build).

## Solid Snake's playable identity

The [1995 Solid Snake character/gameplay doctrine](docs/SOLID_SNAKE_1995_PLAYABLE_IDENTITY.md) contrasts the first-mission rookie with Naked Snake's MGS3 wilderness-survival style while preserving shared CQC knowledge and the source-faithful weapon/prisoner loop. The [eight-trait identity contract](data/solid_snake_1995_playstyle_contract.json) assigns concrete character expressions to each numbered mission. **Character-direction work is not a substitute for real rigging, animation, engine compilation or playtesting.**

## Character stories — 1995 campaign

The [complete 1995 campaign narrative and cast timeline](docs/1995_FULL_CAST_NARRATIVE_AND_TIMELINE.md), [40-sequence storyboard](data/1995_campaign_sequence.json), [individual per-character chronological timelines](data/1995_individual_character_timelines.json), and [22-character/group / 105-beat planning registry](data/character_arc_registry.json) now map the core ensemble from first radio contact through Outer Heaven's finale, including individual boss encounters. This is **narrative design only**, except the separate source-level Gray Fox prison prototype and opening radio/mission code. All character beats remain unverified against the target Connect2Media J2ME edition. Narrative `depends_on` links are *not* hard mission gates. Check with `python tools/validate_character_arcs.py` and `python tools/validate_1995_sequence.py`.

## Guard duty and prison census — game-wide binding rule

The [guard-duty/prison-occupancy doctrine](docs/GUARD_DUTY_AND_PRISON_OCCUPANCY_POLICY.md) and its [machine-readable contract](data/1995_guard_duty_and_prison_occupancy_contract.json) govern **all eight numbered missions**. The current Mission 2 secret prison shows four detainees in the provisional source greybox: Snake, Gray Fox and two noncanonical ambient figures in barred side bays. They do **not** create extra rescuable POWs. The two prison guards now patrol actual cell-observation and confiscated-property posts; seven active Building One guards have distinct real duties. Guards continue working when Snake is on another floor. Actual enemy and prisoner positions for later missions require J2ME source review and UE collision/AI playtests. Structural check: `python -m unittest tools/test_guard_duty_contract.py`.

## Global production doctrine — binding

**Doctrine 000 — [How We Build](docs/DOCTRINE_000_HOW_WE_BUILD.md)** governs every other doctrine. Product quality is defined by the *method*: research and provenance, player-first intent, integrated execution, actual evidence, accessibility, source fidelity, performance and maintainability. Its [Doctrine 005 — Spatial Structure](docs/DOCTRINE_005_SPATIAL_STRUCTURE.md) companion binds topology and room rhythm to the same standards, with [area contracts](data/area_contracts.json). When requirements conflict, record an explicit decision and test the player's experience instead of allowing undocumented shortcuts.

Every player-facing beat must be designed as **one synchronized audiovisual, interactive and temporal experience**, not mechanics first and cosmetic polish later. See [Doctrine 004](docs/DOCTRINE_004_INTEGRATED_EXPERIENCE.md) and the [beat registry](data/beat_contracts.json). Visual foreshadowing, spatial sound, inputs, timing, AI knowledge, durable state, interruptions and accessibility must agree; a polished asset is not proof of a finished beat. The registry begins with nine source/design-stage contracts and is not exhaustive. Local regression checks are required before packaging; Unreal playtesting is still outstanding.

## Local checks

```sh
python -m unittest discover -s tools -p 'test_*.py'
python tools/validate_beat_contracts.py
python tools/validate_area_contracts.py
python tools/inspect_and_extract.py /path/to/Metal-Gear_J2ME_EN_v330.zip --output content
cd rust && cargo test && cargo build --release
```

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
