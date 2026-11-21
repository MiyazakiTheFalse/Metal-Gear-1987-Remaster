# Original 1995 Transceiver Interface — Audio Source

This folder contains five original synthesised **48kHz mono 16-bit PCM** source cues (Incoming, Connect, Tune, Close and Signal). These are newly composed for this project, **not** samples copied from Konami, licensed gun Foley, music or acted radio voice. They are source materials awaiting UE import and mix verification.

Use [the Unreal Editor import script](../../../../unreal/MGRemaster/Content/Python/import_transceiver_audio.py) in the open UE5.7 editor. It validates each WAV and requires a real SoundWave at /Game/Audio/Transceiver/<cue> before logging success. The C++ player loads these exact paths, and silently degrades to accessible readable text if audio is missing.

The opening call currently plays an incoming sequence and connection chime while the player remains in control; manual tuning produces a short cue; closing cancels a pending connection and plays a disconnect sound. No official Metal Gear calling tones or voice acting is used.

**Not completed:** UE Editor import, custom speaker EQ/voice acting, waveform mix, final loudness normalisation, music ducking, 720p/a11y hearing and hardware testing. See [full audit](../../../../docs/VERTICAL_SLICE_ASSET_COMPLETENESS_AUDIT.md).
