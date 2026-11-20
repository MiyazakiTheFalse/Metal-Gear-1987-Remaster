# 1995 Outer Heaven — animation, hair and voice handoff / quality matrix

**Date:** 20 November 2026
**State:** C++ transition and playback source, original provisional hair OBJ and production templates committed; no UE5.7 compile or finished character voice performance, rig, AnimBP graph, groom or full-camera cinematic has been demonstrated.
**Authority:** Doctrine 000 (the quality of how it is experienced), source-first 1995 continuity, real gameplay state over cutscene state.

## Visual and motion handoff, not an asset-completion claim

| Encounter or intent | Source signal currently provided | Artist/engine asset still missing | In-game acceptance |
|---|---|---|---|
| Snake idle → walk/run → stop/pivot/reverse | GroundSpeed, Acceleration, YawVelocity, start/stop/reversal, EMGPerformanceAction | Genuine 8-way BlendSpaces, 45/90/180 pivots, foot IK, stance idle acting | No visible skating or instant standing-body rotation after contact |
| Snake crouch/prone | Validated movement-capsule postures; CrouchTransitionAlpha and ProneTransitionAlpha | Stance enter/exit clips, grounded knee/hand/torso transitions and camera review | No clipping/teleport to an impossible stance; low ceiling clearance respected |
| Cover, peek, shuffle, wall-knock | CoverTransitionAlpha, CoverPeek, WallKnock and physical cover movement | Hand-wall IK, head peek alignment, 1995 outfit/fabric clips, sound placement | Peeking breaks LoS naturally; hands align; knocking is a credible impact, not a phantom wave |
| Aim → fire → recovery / melee | AimPitch/Yaw and real committed recoil, melee/cancel visual timer | Grip/aim additivities, muzzle/hand socket validation, hit reactions, stagger/KO montage | Animation never fires the gun, awards a hit or forces a recovered weapon |
| Snake capture → unconscious → confinement | Confiscation and relocation commit, then 0.82s black-to-cell camera fade | Authentic custody procedure, handcuff/disarm and transport Sequencer, skip-to-same-state checks | After skipping, exactly the same cell spawn, equipment, flags and guard awareness; no lingering camera lock |
| Snake hairstyle / face | Original scalp/locks OBJ/MTL, `head` attachment, artist offsets, outfit-variant preservation | Finished face, bone fit, hair groom/material/LOD and head/neck motion | Hair is visibly on the **same** young Snake in both costumes, fits skull/hairline, no clipping on crouch/turn/cover; must read at 720p |
| Prison guard duty → notice/search/challenge/report | Guard posture/evidence, OnPerformanceActionTransition, delayed/cancellable reports | Individually authored head-first reaction, gait, communications hand movement, voice and patrol turning | No radios from unconscious guards, no premature melee collision, no lip-sync until VO exists |
| Gray Fox prisoner → recognition/rescue | FoxPrisoner / FoxRescued source actions, human-only rig path, optional world VO | Actual exhausted seated-to-standing/leaning performance, mouth/breath/eye line | No cyborg era appearance; intelligible key Metal Gear intel even on dialogue skip |
| Shoot Gunner search → warning → 3 blasts → pump → recovery | GunnerSearch/Aim/Volley/Recovery/Defeated; 0.78s provisional warning, 0.52s shot gap, pump 0.17s later, 1.85s recovery | Body/weapon rig, stock into shoulder, cheek weld, barrel attitude, sight/hip turn, fire/pump/reload animation and layered Foley | Pump NEVER same frame as blast; not instantly rotated; remains fair with all subtitles and sound muted |
| Radio listen while moving | Additive RadioListenOverlayAlpha and voice scene ID | Transceiver hand/ear/face overlays, UI portraits, no movement-lock montage | Snake can move, retune and close immediately; no dialogue/pause gate |

The animation event callbacks **observe the simulation** and may start/stop montages, facial curves and FX only. They must not commit mission flags, apply hits, unlock doors or feed guard knowledge. Character motion is currently in source-only state; these are acceptance requirements for future imported assets, not a promise that clips exist.

## Dialogue and mix direction

The source manifest covers **49 subtitle rows / 45 naturally spoken cues**, of which **none** is yet recorded. Four split radio screen captions intentionally form a single uninterrupted take. Six independently cast voice profiles: young Snake (contained and inquisitive), real Big Boss (authoritative but not cartoon villainous), Schneider (practical/uncertain), human Gray Fox (worn but precise), Shoot Gunner (proud, heavy and grounded), guards (procedural and state-driven).

- Dry authorised performances are recorded, approved and saved in the named raw paths. Don't use franchise/ripped actor voices, celebrity clones or synthetic imitations.
- Source mastering uses a documented two-pass FFmpeg loudness normaliser, conservative EQ and gentle compression. Radio: target **-18 LUFS**, world speech **-19 LUFS**, both **≤ -1.5 dBTP**, delivered as 48 kHz mono PCM. These numbers are production starting points, not a passed mix evaluation.
- Import mastered WAVs into `/Game/Audio/VO/<speaker>/<clip>`. Source playback begins on actual scripted dialogue events; the radio voice fades out/cuts at retune/close/skip, never delays movement or the authoritative event.
- Validate measured integrated loudness and peaks after render, consonant intelligibility over weapon/music/storm noise, a final SoundClass/submix ducking/limiter chain, silence-safe subtitles, input/muted/controller compatibility and near/far-world spatial presentation.
- Actor-specific mouth shapes / jaw and look-at must be timed to the **performed** take. In absence of VO, do not play random mouth loops that falsely imply speech is playing.

## Quality gates before treating this as a vertical-slice build

1. Execute `python -m unittest discover -s tools -p 'test_*.py'`, Rust cargo tests and all asset validators locally; correct any assertion/build regression.
2. On a UE5.7 host compile and inspect the new anim instance, head-socket connection, trigger events, optional voice asset load paths and cooked content. Import original hair and actually fit it to the specific Snake rig.
3. Build real AnimBlueprint graphs and animation clips for each performance action and its interruption, including turning, aim/cover hand contact, low-profile movement, Fox fatigue and Shotmaker stock/pump timing.
4. Capture authored VO with distinct actors, then loudness-process/import/listen, with proper dialogue/music ducking and player-controlled radio cancellation. Approve named voices against the character bibles.
5. Record 720p gameplay: early capture, escape, interrupted guard report, third shotgun pump, skipping Fox and the opening call, hair under cover/prone/in both costumes; capture profiling and first-time player notes.
6. Keep PR #3 **draft** until these are actually evidenced. A source-level fade, mesh hook, script or placeholder is not a finished cinematic, hairstyle, animation or professionally voiced game.

**Honest current inventory:** original hair source exists; the final licensed recognisable Solid Snake head/skeleton, groom shading, animations, voice masters, professionally mixed in-game audio and Sequencer content do not.
