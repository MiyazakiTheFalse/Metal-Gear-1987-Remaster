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

## Local checks

```sh
python -m unittest discover -s tools -p 'test_*.py'
python tools/inspect_and_extract.py /path/to/Metal-Gear_J2ME_EN_v330.zip --output content
cd rust && cargo test && cargo build --release
```

## Windows packaging

On a Windows machine with UE5.7, Rust MSVC, Visual Studio C++:

```powershell
.\scripts\build_windows.ps1 -UnrealRoot 'C:\Program Files\Epic Games\UE_5.7'
```

The resulting `dist/Windows/` package must be smoke-tested before distribution. No UE-capable runner is currently connected; **no executable has been generated**.

The prison's destructible mission walls intentionally **do not** trigger automatic cover: E/X breach interaction retains priority. Knocking is a sound-location distraction, not a direct player-position reveal. See [wall/knock implementation contract](docs/WALL_KNOCK_AND_BREACH_CONTRACT.md). No UE5.7 build validation or finished skeletal animations are claimed.

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
| Swap shoulder | Q | Press right stick |
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
model (14/35-second optional hints, quiet narrative beat, escalating escape).
The Unreal scene has a reduced fallback state machine for iteration without Rust.
See [pacing doctrine](docs/DOCTRINE_003_PACING.md) and
[rescue implementation notes](docs/GRAY_FOX_RESCUE_IMPLEMENTATION.md).

**Still missing:** verified J2ME prison map and script, final Gray Fox/Snake
animations, real prison art, authentic scene audio, proper AI navigation/combat,
and compilation/playtesting on an Unreal-capable workstation. Source is not yet
a downloadable executable.
