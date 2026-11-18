# DOCTRINE 004 — Integrated Experience and Temporal Coherence

**Status: BINDING, GAME-WIDE.** Applies to every player action, environmental affordance, AI response, mission event, camera transition, menu, radio exchange, cutscene, tutorial and checkpoint. Complements Doctrine 002 (events) and Doctrine 003 (pacing); it does not override story/source verification.

## Non-negotiable rule

**Visuals, audio, interaction, consequences and timing must be conceived, implemented, tested and reviewed as one coherent player experience — never as independent finishing passes.** A mechanically functional action with missing, misleading or mistimed presentation is unfinished. A polished audiovisual sequence that blocks or contradicts player agency is unfinished.

Design from the player's experience: What do I notice? What can I infer? What can I do now? What happens when I act, and when will I see/hear/feel the result? What remains true if I skip, interrupt or replay it?

## Beat contract — required for every shippable feature

Each discrete interaction or authored beat has a unique stable ID and a single record with:

1. **Provenance and intent:** source reference/status (verified original, later-canon comparison or remake addition); narrative purpose; desired player inference/emotion; connection to the previous and next beat.
2. **Trigger and agency:** eligibility, environmental prerequisites, available player actions, exactly when control is retained/lost/restored; one or more fair ways to discover critical clues.
3. **Visual language:** pre-action foreshadowing (silhouette, contrast, staging, damage, readable animation), event/impact, recovery/aftermath and camera collision; never reveal unseen rooms through the camera.
4. **Audio language:** world source, spatial distance/occlusion, material response, room tone, VO/music hierarchy, subtitle equivalents and the player's ability to identify the stimulus.
5. **Temporal design:** trigger moment, causal ordering, input-to-confirmation latency, animation/audio synchronization, overlap and interrupt windows, optional clue escalation and reset/return timing. Distinguish authored *targets* from measured results.
6. **Durable state and AI:** atomic event flags, environmental persistence, inventory, actual sound/sight knowledge, AI action and return state. Presentation cannot be the owner of mission progression.
7. **Failures and alternatives:** move away, repeated input, being spotted, death, stun, sprint, pause, skip, save/load, audio unavailable, low frame rate, gamepad and remapped input; no softlock or fabricated enemy knowledge.
8. **Proof:** unit/integration tests for state/order, packaged gameplay captures with synchronized input/audio/video, 30/60/120 fps and frame-time measurements, first-time/repeat-player comprehension, and source/rights sign-off.

## Production sequence (definition of ready and done)

- **Ready to implement:** approved beat contract showing the sight/sound/action/time/state design simultaneously, plus dependencies, player-driven deviations and provenance. Mark unknowns explicitly instead of guessing lore.
- **Prototype:** use genuine causal triggers and instrument cue timestamps, even with temporary visuals and sounds. Stand-ins must be labelled. No simulated 'finished' feedback without the underlying event.
- **Implementation:** implement event transaction separately from presentation; dispatch visual/audio/haptic/subtitle/AI cues against a stable event ID. Each channel can fail gracefully; none may commit the same flag a second time.
- **Review:** do an end-to-end pass in order, then deliberately play out of order and at different speeds. Do not approve individual animation, SFX or AI code in isolation if the experienced beat is incoherent.
- **Done:** compiled, packaged, photographed/recorded proof shows synchronized channels, sensible pacing and accessibility; all blocking defects resolved. A document, test file or code diff alone is NOT gameplay verification.

## Timing and pacing requirements

- A player who understands a puzzle may act immediately: no compulsory dwell time added to stage hints, VO, camera moves or sound.
- Reaction precedes explanation. Telegraph danger and ensure input confirmation promptly, then allow follow-through and aftermath without artificial animation locks.
- Make time budgets explicit in the beat registry. Proposed numbers are tunable hypotheses, not claims about 'canonical' timings.
- Set time windows relative to actual gameplay events, not wall-clock cutscene starts. Optional hint timers reset when meaningful progress occurs; hints suppress on alert and cancel when solved.
- Time-dependent behaviour must remain stable with variable delta time, pauses, time dilation and reload. Dialogue playback must never hold a committed state hostage.
- Prefer layered sound and plausible silence; no indiscriminate stinger on every action. Sound heard by guards must have an actual location and attenuation, distinct from a UI notification.

## Accessibility is part of the same design

Every critical sound clue needs a non-audio route (surface wear, subtitles/captions, reaction, visual inspection). Every critical visual clue needs a non-colour-only cue. Preserve subtitles for radio and guard chatter. Camera shake, flashing, subtitles, audio mix and control mappings must be adjustable. Avoid forcing private-ear communications to spatialize as though broadcast across the room.

## Gray Fox prison reference implementation — source is still provisional

- **Capture/regain agency:** visible disarm, equipment state transaction, cell ambience, playable movement immediately after capture checkpoint. Preserve why Snake chose capture through a cinematic/briefing beat.
- **Observation/architecture seed:** optional Kyle Schneider transceiver contact describes changed passagework without knowing the secret prison's exact cell. No compulsory radio call and no instant interaction marker.
- **Discovery:** mismatched patch texture, conduit, crack or hollow response distinguish the candidate wall from ordinary walls; acoustic and visible evidence must agree. Additional clues may appear after inactivity without gating progress.
- **Breakthrough:** E/X remains available as soon as range and original mission prerequisites are met; impact/contact, cracks, debris, acoustics and final collision removal must agree with the committed event. Source greybox currently removes the wall immediately; staged damage/audio are a future production requirement, not implemented art.
- **Gray Fox encounter:** his fatigue, camera, quiet ambience and dialogue priority earn a slower perceived moment without mandatory waiting or forced enemy spawn.
- **Exit/recovery:** second breach creates a genuine, audible/visible route and allows guard investigation only when the sound can plausibly reach them. Patrol, suspicion, alert, escape and recovery transitions reflect actual guard knowledge.

## Enforcement

- Register designed beats in data/beat_contracts.json and validate them with tools/validate_beat_contracts.py; include all critical fields and label status truthfully.
- Adding a gameplay event, interaction, AI stimulus or cinematic without a contract is a review blocker, not an excuse for an audio/animation polish backlog.
- No beat can be marked 'verified' until Unreal compilation, capture, instrumentation, and human playtesting confirm it. Prototype contract status cannot be promoted by static tests alone.
- Each PR touching player-facing behavior should state affected beat IDs, sight/audio/interaction/timing changes, interruption cases, and remaining missing assets/tests.

## Review question

**At this exact instant, does what the player sees, hears, can do, and understands all agree with what the game is actually doing — and are the pauses and transitions earned?** If not, the beat is not ready.