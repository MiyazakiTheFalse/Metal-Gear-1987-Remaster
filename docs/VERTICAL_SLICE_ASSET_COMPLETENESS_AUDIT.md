# Radio and complete vertical-slice asset audit — 19 November 2026

**Truthful status:** repository GitHub tree was inspected on `feature/character-art-pipeline-1995`. Prior to this pass it contained only **three** binary WAV sound files (Shotmaker aim, blast and cycle), an editable shotgun OBJ/MTL, seven greybox OBJ models and zero verified `.uasset` files. We have since authored **five original** transceiver WAV cues, but they are still **source files**, not proof of a playable imported/compiled sound design.

## Current transceiver audio assets

| Cue | Behaviour in C++ | File source | In Unreal? |
|---|---|---|---|
| Incoming | Once at fresh N313 call, while player moves | Transceiver_Incoming.wav | **No** |
| Connect | Once after opening motif, only if player hasn't closed/tuned | Transceiver_Connect.wav | **No** |
| Tune | Every manual Q channel change | Transceiver_Tune.wav | **No** |
| Close | Every deliberate C exit | Transceiver_Close.wav | **No** |
| Signal | Short dry radio hiss/contact on manual call | Transceiver_Signal.wav | **No** |

All are originally synthesised mono 48,000 Hz 16-bit PCM, with a short envelope and conservative levels. They are **not copied Metal Gear communications tones**, voice lines or high-end recorded transceiver Foley. The source WAVs are in `assets/source/transceiver/audio/`; `unreal/MGRemaster/Content/Python/import_transceiver_audio.py` imports them as SoundWaves at the code's exact `/Game/Audio/Transceiver/` paths.

The N313 introduction remains optional to watch/listen through: closing cancels the pending connect chime; skipping does not lose mission intel. Call interaction cannot prevent movement, initiate damage or wait on audio completion. Radio VO is **not** yet present.

## Is the vertical slice fully resourced?

**No.** The machine-readable [inventory](../data/vertical_slice_asset_inventory.json) lists 40 concrete required assets or work packages across cast, animation, props, environments, audio, music, cinematic and UI systems. Current classifications: scripted_reference: 9, missing: 20, prototype_source: 11. Everything is currently `imported_into_ue=false` and `production_approved=false` because no imported Unreal content files or captured build evidence were found.

**Most critical missing pieces:** recognisable fully rigged 1995 Snake and Gray Fox; guards and Shoot Gunner rig/animation (including physical nonlethal arrest and recoil/pump); cinematic character acting and Sequencer; original score and dynamic mixing; named VO performances; coherent final environment geometry, materials, lighting and destruction VFX. Prototype OBJ mesh and synth sounds do not meet this quality bar.

## Quality-first asset roadmap

1. **Radio continuity, highest near-term leverage:** import existing five WAVs, hear in Editor and test open/tune/close timing while moving. Record a direct output capture before calling them implemented in game.
2. **Recognisable character first:** commission/create/legally acquire the actual skinned 1995 hero and Fox, retarget correct motion, and render them **in gameplay light**. Do not pass off Quaternius/Mixamo stand-ins as Solid Snake.
3. **World scale and tactical readability:** complete an internally coherent detention facility and custody route; guards observe, challenge, search and arrest with a proper performance. Art materials and environmental audio should agree on surfaces.
4. **Shoot Gunner identity:** finish a large-gauge riot shotgun with actual mesh sockets and animation, bespoke body/face, weapon-cycle sound and accessible shot telegraph. The original MSX2 spread informs gameplay; specific shotgun model is not canon confirmed.
5. **Original music and full sound mix:** author original motifs, separate stems for stealth, suspicion, capture, imprisonment, reveal, boss and aftermath. No copyright-uncleared Metal Gear soundtrack, rips or impersonated VO. Implement state-aware crossfades and speech intelligibility.
6. **Final verification:** legal permission and provenance, imports and in-editor paths, animation contact, camera/skip parity, 720p coherent visuals, controller and no-audio options, packaged performance and unfamiliar-player research.

This audit is an accurate list of **what still needs to exist**, not a claim that the authoring tool, a candidate on a website or the current C++ load path constitutes an asset in a game build.

## Full repo task

- `python tools/validate_slice_asset_inventory.py` checks asset paths and explicit imported/approval states.
- `python -m unittest discover -s tools -p 'test_*.py'` runs local Python contract tests.
- Open UE 5.7 Editor and run the import script. This has not yet been executed by the connected GitHub environment.
