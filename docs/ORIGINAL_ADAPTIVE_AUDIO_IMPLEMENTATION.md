# Original Adaptive Audio Implementation — Both Engine Integration Pass

Updated: 21 November 2026.

## Actual deliverables

An independently authored, no-extracted-samples audio bundle was generated in the current working environment:
- **14 new MIDI arrangements**; these are **not transcriptions** of the copyrighted Metal Gear soundtrack.
- **14 rendered looping music tracks** as 32 kHz PCM16 mono WAV.
- **46 original sound effect WAVs** (no Konami audio extraction).
- A SHA-256 manifest, original deterministic audio generator, and asset integrity regression tests.
- A Gray Fox browser playtest ZIP **containing the actual WAVs** and an adaptive WebAudio runtime.
- An Unreal audio overlay ZIP **containing the same WAVs**, suitable for import through this repository's `tools/unreal_import_original_audio.py`.

These downloadable binaries are **not in the GitHub branch**. Copy the ZIP's contents into your local repo before cooking Unreal. The current GitHub branch has the new C++ director and the import script. A source-code hook alone is not evidence of a live sound asset.

## Unreal source integration

`AMGAdaptiveAudioDirector` is spawned by `AMGRemasterGameMode` even for the title screen, and returns on Continue. The director maps cues to `/Game/Audio/Music/<id>.<id>` and `/Game/Audio/FX/<id>.<id>`. It selects music from:
- actual mission index and campaign story beats;
- geographically local guard awareness, never magical off-floor enemies;
- boss proximity and the source-specific TX-55/escape beats;
- Game Over or Title Screen, overriding combat tension;
- real Codec opening and closing, with music ducking.

Crossfades run between separate audio components. A finite cue cooldown limits repeated UI and footstep playback, while step material is inferred from floor geometry/tags. The SFX feedback path calls the director before falling back to legacy `/Game/Feedback` hooks. Real melee contacts use separate punch-one, punch-two and kick cues; choke/drag/locker, mine, grenades, rockets, boss windups and attack releases are distinct events.

**Engine setup:**
1. Extract `OuterHeaven_Unreal_Audio_Overlay_v1.zip` at the repository root. Files go into `unreal/MGRemaster/ArtSource/Audio/{Music,FX,MIDI}` plus `audio_bank.json`.
2. Enable Unreal Engine's **Python Editor Script Plugin**.
3. Open the UE 5.7 project and run `tools/unreal_import_original_audio.py` from within the editor; it checks hashes and imports into `/Game/Audio/`.
4. Inspect the imported SoundWaves: ensure **Looping** is enabled only for music, and regenerate/cook for release.
5. Run PIE and listen at each mission state (not just the editor Content Browser). This has **not been performed here**.

## Browser source integration

The separate playable browser ZIP includes `audio_manager.js` and `assets/audio/Music/*.wav`, `assets/audio/FX/*.wav`. Its script is loaded before `game.js`. The manager:
- loads decoded WAV assets asynchronously once the user initiates audio;
- resolves the same stable cue IDs and aliases as Unreal;
- crossfades ambience/infiltration/caution/alert/boss/Game Over themes without resetting on every frame;
- adds stereo placement and distance attenuation to world SFX;
- reduces background music when a dialogue/Codec line is being presented;
- exposes independent **Music** and **Effects** sliders in the pause menu;
- retains procedural-synthesis fallback if loading fails or file:// restrictions apply.

The browser bundle is **the current Gray Fox vertical slice**, not the complete eight-mission game. Its original one-hit melee remains a gameplay parity limitation even though the sequential punch sound names exist; the more sophisticated punch–punch–kick logic is in Unreal source.

## Music and effects coverage

| State | Source WAV |
| --- | --- |
| title | `music_title.wav` |
| searching/infiltration | `music_infiltration.wav` |
| suspicion | `music_caution.wav` |
| confirmed alert | `music_alert.wav` |
| captured / prison | `music_prison.wav` |
| reveal | `music_revelation.wav` |
| proximity to authored boss | `music_boss.wav` |
| Building Two research | `music_research.wav` |
| desert | `music_desert.wav` |
| disillusionment | `music_betrayal.wav` |
| TX-55 approach | `music_metalgear.wav` |
| countdown escape | `music_escape.wav` |
| death | `music_game_over.wav` |
| ending | `music_ending.wav` |

Effects include: three authentic sound-grouped **distinct attack events**, choking, dragging, lockers, broken wall, metal/soil/concrete footfalls, weapon fire categories, mine planting, rocket/grenade launches, card locks, electronic interference, radio, failure sting, ambient industrial machinery, guards and boss behaviours.

## Licensing and production quality gates

Generated music and effects are original, but are **procedural first-pass recordings**, not a replacement for a bespoke composer, field-recorder or recorded character performances. The produced MIDI arrangements are **not** the J2ME source game's protected compositions and are never presented as upgrades of those tracks.

To call the whole audio experience *production ready*, a tester must still finish:
- Unreal Editor SoundWave import and compiled PIE/cooked-playback verification;
- volume/loudness standardisation and headroom on headphones, TV and handheld speakers;
- every loop seam, alert crossfade, repeated SFX cooldown, spatial attenuation and no clipping under 12+ sources;
- musical identity/arrangement approvals and acoustic texture replacements for the full 1995 story;
- mission transitions, especially Gray Fox, Jennifer, Ellen, TX-55 and Big Boss;
- audio settings persistence, subtitles/captions for critical events, mono compatibility, memory/CPU budget and asset redistribution review;
- browser gamepad testing on supported browsers and Unreal package smoke tests.

The original source's 56 J2ME MIDI files remain privately held in the user's supplied archive and are **not included in any distributable pack**.

For the current audit see [1995 audio coverage](../data/1995_audio_coverage_audit.json).
