# Gray Fox Prison — Integrated Beat and Cue Sheet

**Status:** Binding design/verification checklist under Doctrine 004. Visual timings below are **proposals unless explicitly marked source-wired**. The source-only prototype has no final models, recorded VO, authored animation montages or sound assets. Not a claim of Unreal playtesting.

## How to read cue timings

- Every time is relative to a gameplay trigger, not a compulsory elapsed timer from scene entry. Where a target is provisional, tune to recorded first-time player behaviour and frame-time budgets.
- A valid input must commit gameplay state independently of SFX, animation, subtitle or camera availability. Actual gameplay visual/sonic confirmation should meet the registered feedback budget once real assets exist.
- Radio never requires player attendance. A skipped or interrupted sound must never erase a clue, a rescue flag or an actual guard hearing event.

| Beat | Player-led trigger | Visual and animation sequence | Audio and spatial response | Timing/control contract | Current truth |
|---|---|---|---|---|---|
| Capture handback | Capture checkpoint loaded | Snake disarmed, cell silhouette, neutral playable camera | Room tone and clothing/metal gear clinks | Control available immediately at current checkpoint; no forced tutorial dwell | **Source:** flags + instant cell spawn; **Missing:** capture cinematic, disarm animation, audio |
| Schneider architecture seed | Player opens C/Start, selects 120.79 | Text-first transceiver presentation; later 1995-appropriate portrait and listening pose | Local receiver tone/voice; mix must not mask guard footsteps | No auto-call, no pause, replayable. Channel tune Q/RS | **Source:** optional HUD text; **Missing:** character art/VO/ducking |
| Look for repaired masonry | Player examines cell, moves or experiments | Distinguishable patch edge and construction clue; no luminous quest arrow | Different solid/hollow impact responses, optional caption/visual equivalent | First wall interaction always eligible; 14s then 35s are current *progress* hint thresholds, not verified player-idle tracking | **Source:** non-spoiling objective + hint timer; **Missing:** seam art and localized tap audio |
| First breach | Player E/X within wall range | Impact, escalating visible fracture, dust, collision-open consistent with state commit; no floating seam props | Debris and fracture at the exact wall, room reverb changes when opened | No animation gate before accepted action; animation event may trail state, never retroactively deny entry | **Source:** one E immediately destroys wall and commits flag; **Missing:** progressive cracking, debris, material audio |
| Gray Fox reveal | Player reaches and interacts | Captive's distinct posture/face, readable reaction, held silence framed without stealing camera | Dialogue/intel prioritized over ambience, no fabricated guard response | Existing subtitle duration 11s, but controls **not locked**; final VO timing remains to be authored | **Source:** rescue transaction + temporary subtitle, not a performance |
| Exit breach | Player E/X near second weak wall | Second opening reveals guarded route, matched mesh/collision and debris | Sound can propagate to guards only if physically plausible | No forced delay after rescue; clearance and sound events depend on real state | **Source:** instant second wall removal; **Missing:** stage/AI sound propagation for actual breach |
| Wall knock | Player F/A when attached to ordinary wall | Hand/contact anticipation, real wall impact, settle | Material-dependent localized knock; sound stimulus is wall impact, not Snake transform | **Source:** 0.70s input cooldown, 0.30s proxy feedback timer; sound assets currently absent | **Source:** event and AI stimulus; **Missing:** skeletal motion, authored impacts |
| Guard hears a knock | Knock within hearing/occlusion threshold | Guard stops, turns, walks, scans, returns to patrol | Spatial guard acknowledgement/boots, music only if perception justifies it | **Source:** initial reaction 0.22–0.45s, search 2.5s plus 0.85s per repeated knock; movement distance and collision determine total duration | **Source:** collision-swept prototype investigation; **Missing:** route-aware NavMesh and actual acting/audio |
| Escape/recovery | Player reaches exit after breach | Saved state, exit feedback and calm end of scene | Tension resolves when actual threat ends, never by hiding still-alert guards | Checkpoint commits once immediately at exit; no forced victory camera | **Source:** completion flag/checkpoint; **Missing:** performance/audio/lighting pass |

## Temporal failure matrix

1. **Immediate solver:** start in cell; player identifies wall instantly and breaches. No Schneider or Big Boss contact is required, and neither can stop the interaction.
2. **Curious solver:** explores, notices a physical difference, then optionally calls Schneider. The radio should enrich the deduction rather than reveal an invisible interactable.
3. **Stuck solver:** after 14s without mission advancement, a subtle cue may appear; after 35s a stronger, still non-spoiling cue. These are current heuristic defaults, not evidence-based thresholds. The existing implementation does not detect all meaningful exploratory actions.
4. **Speedrunner/replay:** rapidly breach/meet/breach/escape; no cue waits, duplicate flag, radio requirement or hardcoded wall-delay. Test at 30/60/120 fps.
5. **Combat interruption:** if a guard genuinely sees Snake, its alert takes priority over a noise distraction. Subtitle, rescue transaction and checkpoint remain coherent if a radio panel is open.
6. **Accessibility:** players with audio muted must see the material difference or other readable non-audio equivalent; hearing-impaired players should have captions for knock/footstep cues. Players with reduced visibility must not rely on hue alone.
7. **Resource failure:** missing VO/SFX/animation cannot prevent flag/AI transaction. The game still needs honest visual feedback and accessibility to be considered production-complete.

## Review evidence before sign-off

For each beat, save an input-stamped capture containing player action, world/AI state changes, animation notifies, HUD subtitle onset/offset, audio playback onset, camera state and frame time. Record expected versus observed latencies, comprehension, first-time dwell, clue recognition, mission progress and skip/reload behaviour. File results against the beat ID in data/beat_contracts.json; never promote 'unverified' merely because the document exists.

**Present blockers:** Unreal compile/build, licensed rigged characters, crack stages and debris, distinct sound assets and actual audio mixing, guard NavMesh, real telemetry/captures, first-time user testing and source/J2ME room verification.
