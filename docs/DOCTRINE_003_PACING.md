# DOCTRINE 003 — Mission Pacing and Dramatic Agency

**Status: BINDING.** Applies to cinematic and emergent events, stealth spaces, camera direction, encounter difficulty, and checkpoint placement; complements DOCTRINE 001 and 002.

## The rule

Game-wide [Doctrine 004](DOCTRINE_004_INTEGRATED_EXPERIENCE.md) requires audiovisual cues, control affordances, event state and their exact relative timing to be designed and validated together for every beat; the pacing doctrine cannot be satisfied by a timing-only checklist.

**Pace the experience through space, information and consequence, not artificial waiting.** Design the intended tension curve but let the player vary speed and tactics. Preserve original story/event dependencies. If the game forces a stop, it must be needed for a meaningful interaction, not for a timer or animation budget.

### Gray Fox prison vertical slice

| Phase | Player feeling | Design intent | How implemented |
|---|---|---|---|
| Capture | Shock | One brief loss, regain agency | First mission-state transaction confiscates equipment; spawn in cell |
| Cell discovery | Vulnerability / curiosity | Safe observation, clues, camera control | First weakened wall can be breached immediately; hint increases if stuck |
| Breakthrough | Short exertion | Fast, tangible progress | Breach reveals adjacent cell; player moves freely |
| Revelation | Narrative emphasis / release | Time to absorb Gray Fox's intelligence | Rescue commits once; no automatic unrelated reinforcement wave or forced wait |
| Escape | Rising threat | Consequence-driven urgency | Second wall opens the route; guard pressure resumes, player chooses tactics |
| Recovery | Relief | Let story resonate before next challenge | Milestone records completion; avoid instant extra threats |

### Behaviour constraints

1. **No compulsory dwell times.** The first wall and rescue interactives are actionable as soon as prerequisites are met. Narrative cue playback must not gate the state transaction.
2. **Hints only after inactivity.** First soft environmental nudge after 14 seconds without progress, stronger non-spoiler hint after 35 seconds. Values are tuning hypotheses and require playtests; never overlay during a real guard alert.
3. **Earned quiet.** The revelation phase should not spontaneously spawn unrelated combatants. It does not magically erase guards who actually saw or heard Snake.
4. **Play-space is pacing.** Player cell supports observation; first breach is short; Fox cell has room to stand and hear dialogue; escape route supports cover, camera clearance and an approaching threat. Verified J2ME geometry remains source-of-truth; greybox coordinates are provisional.
5. **Interrupt safely.** If danger, death, save/reload or fast repeat interrupts a cue, committed flags remain correct; uncommitted state must roll back.
6. **Restraint in presentation.** Use one distinct cue per revelation, not a cascade of pop-ups. Target clear voice priority, subtitle readability and no unnecessary camera seizure.
7. **Pace isn't fixed runtime.** Record first-time and repeat playthroughs, skill levels, failure/stuck time, duration in each beat, camera collisions, awareness and player-led deviations; adjust topology, affordances and hints based on evidence.
8. **Respect player tactics.** Stealth success may feel quiet; detection creates legitimate pressure. Do not rubber-band enemy awareness simply to force the intended emotional arc.
9. **Allow exploration.** Optional environmental details should reward attention without becoming critical-path chores.
10. **Finish with a clean checkpoint.** Avoid making players replay the entire sequence after a late failure.

### Engineering policy

- Rust \`gray_fox.rs\` owns ordered event flags; \`pacing.rs\` maps committed flags and time since last advance to optional pacing guidance; neither renders nor spawns enemies.
- Unreal owns geometry, camera, prompts, audio, animation and guard behaviour; it must not treat a target-intensity number as an order to fabricate detection.
- Progression is independent of elapsed time. Timing only affects optional hints and presentation.
- Test: correct order, no duplicate rescue, all steps usable immediately, rising/relaxing intensity phases, hint thresholds, suppression during alert, save/load no replay.
- Status: implementation source in progress. Unreal integration and playtests are **not** certified by document creation.

## Rationale and references

- [The Level Design Book — pacing](https://book.leveldesignbook.com/process/preproduction/pacing): distinguish authored pacing from emergent player action.
- [The Anatomy of a Stealth Encounter](https://www.gamedeveloper.com/design/the-anatomy-of-a-stealth-encounter): emotional variation and enemy placement influence stealth encounter difficulty.
- [Hideo Kojima, TIME interview (2014)](https://time.com/2862423/interview-hideo-kojima/): tension release and changes of mood are deliberate ingredients in Metal Gear.
- [GDC 2017 Level Design Workshop](https://www.gamedeveloper.com/design/gdc-2017-level-design-workshop): early blockout and encounter testing for multiple play styles.

**Acceptance:** a polished animation is never proof of good pacing. The sequence must be playable with mouse and gamepad, no mission dead-ends, and recorded evidence of sensible tempo on first and repeated attempts.
