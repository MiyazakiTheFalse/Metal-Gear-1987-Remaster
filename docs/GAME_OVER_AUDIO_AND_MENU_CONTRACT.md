# Game Over — 1995 Metal Gear Sound and Menu Contract

**Status:** Unreal C++ and Canvas UI implemented in source on feature branch; **not compiled or playtested**. No finished music, voice, audio files or authentic artwork imported.

## The player-facing experience

- When LIFE reaches zero: Snake stops moving and stops changing stealth/equipment state; the world clock is returned to 1x. Camera remains on the last image until the dark failure overlay.
- Immediately show **GAME OVER** in a distinctive red-on-green tactical readout. The game does not auto-reload.
- At 0.35 sec, display a short mission-relevant radio distress line and optional non-extracted radio cue. Do not reveal the Big Boss plot twist early.
- At 1.65 sec, enable a visible **CONTINUE / EXIT TO TITLE** choice. Continue is highlighted by default to prevent accidental loss of momentum.
- **CONTINUE** reloads the current level and rehydrates current mission, POWs, recovered weapons, consumables and discovered rooms from the existing campaign and mission checkpoint slots. It never invokes ResetForDevelopment or deletes saved progress.
- **EXIT TO TITLE** returns to an in-session title screen, without spawning game mission actors behind the menu. The title screen offers **START / CONTINUE** and **QUIT GAME**.
- All text is visible if audio is off. Keys Up/Down or gamepad D-pad select; Enter, E or gamepad bottom face button confirm. No mandatory mouse.

## Sound cues — placeholders NOT files

| Cue ID / implementation reference | Intended new audio | Status |
| --- | --- | --- |
| `/Game/Feedback/game_over_sting` | Original abrupt failure sting / radio interruption | C++ hook only, missing sound file |
| `/Game/Feedback/game_over_radio` | Brief waveform/static/radio chirp | C++ hook only, missing sound file |
| `/Game/Feedback/menu` | Choice tick | C++ hook only, missing sound file |
| `/Game/Feedback/select` | Confirmed action | C++ hook only, missing sound file |
| `continue` thematic cue | Level re-entry musical resolution | No arrangement/render/implementation |
| `playerout` thematic cue | Source reference for failure | Original J2ME MIDI in user-owned archive, not upgraded, not in GitHub |

The original 1987/2012 game's musical motifs and source MIDI are copyrighted. This project's public distribution must use properly licensed music and recordings, not blindly ship extracted MIDIs.

## QA before accepting implementation

1. Take lethal damage in Mission 1, 2, 3 Building One, 3 exterior, 4 and 5: fail screen appears, no immediate reload, and the appropriate objective comes back on Continue.
2. Continue never loses collected weapon cache, Card 3, Jennifer rockets, radio intel or rescues. Death before the first checkpoint returns to insertion.
3. Gamepad and keyboard menu controls work even though movement is locked; title selection is never interpreted as firing a weapon or an in-game item use.
4. Accessibility: no critical information solely in radio sounds; visible high-contrast options work at 720p, ultrawide and mono/no-audio.
5. No save deletion or global-time slow motion survives death/reload.
6. Audio files are actually imported, have independent rights tracking and are auditioned in Unreal; volume ducking and fading validated.
7. Realistically rigged death reaction, visual chromatic flicker and fade remain future art/animation passes.

**Broader sound programme:** [full eight-mission audio audit](../data/1995_audio_coverage_audit.json).
