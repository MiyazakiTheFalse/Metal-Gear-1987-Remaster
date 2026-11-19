# SHOTMAKER PROCEDURAL AUDIO SOURCE (2026-11-19)

Three ORIGINAL mono PCM 16-bit WAV source candidates at 22,050 Hz, generated deterministically from synthesized transients, filtered noise and low oscillations. **Not** copied from any Metal Gear release, commercial SFX library or firearm recording; no copyrighted sound source involved. Author: OpenAI, for this project.

- Shotmaker_AimCue.wav — low mechanical tension/metal tick marking committed aim warning
- Shotmaker_Blast.wav — broad, low transient impact/noise, short bass tail for shotgun blast
- Shotmaker_Cycle.wav — paired mechanical rack/slide impulses

These are **source prototypes**, not production-mixed cinematic sounds. They do not prove correct spatialisation, mix loudness, haptics or UE import. Avoid making blast sound disproportionately loud: accessibility and tinnitus-safe playback require actual mix measurements and hearing tests. When editing, replace or layer with properly licensed gun/cycling Foley where possible; never steal the franchise's audio.

To use: import each as a SoundWave into Unreal Engine 5.7 at /Game/Audio/Shotmaker/ with names identical to these WAVs. The C++ boss actor checks that path; if imported the audio plays spatially at the gunner/muzzle during aim, each of his three shots and recovery. If not imported, HUD cue still communicates the actual warning.

Source geometry and acting: ../riot_gun_proxy.obj and docs/CHARACTER_CAST_BIBLES/SHOOT_GUNNER.md . Final audition: quiet and loud listening, night mode/headphones, multiple distances in prison and recovery room, subtitle/visual alternatives, no shot damage while cinematic camera owns control.
