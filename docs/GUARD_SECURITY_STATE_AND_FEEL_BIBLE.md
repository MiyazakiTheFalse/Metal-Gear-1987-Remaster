# GUARD SECURITY STATES, ACTING AND WORLD RESPONSE — IMPLEMENTATION BIBLE

**Status:** first-pass C++ behaviours committed to `feature/character-art-pipeline-1995`; real Unreal compilation, final animation, voice acting and playtesting remain outstanding. The state hierarchy is a **modern remake interpretation**, not a claim that the 1987 MSX2 machine had this AI.

## Why one alert bar fails

A human being can hold a duty, entertain a suspicion, make a tactical decision and react physically *at the same time*. Calling each of those simply 'Alert' creates confusing animation, implausible telepathy and sudden shifts between ignorance and perfect knowledge.

We therefore maintain four layers inside a guard plus one installation-wide response. They must all be legible in the world:

| Layer | States | Consequence |
|---|---|---|
| Duty | Patrol, Prison, Custody | Route, responsibility, station and default body language |
| Local awareness | Routine, Suspicious, Investigating, Confirming, Engaged, Searching, Returning, Incapacitated | What this guard currently believes |
| Evidence | Heard movement, structural impact, direct sight, colleague's sighting, body, colleague's body report, wound | What the belief is founded on; location and age |
| Tactical/acting posture | Patrol, Notice, Investigate, Search, Challenge, Report, Engage, Strike, Incapacitated | Animation cue, sound bark, visible action; cannot damage by itself |
| Facility | Normal, Caution, Alert, Evasion, StandDown | Overall security procedure once evidence is communicated |

### What each local state should look and sound like

**Routine.** A soldier does a job: checks a latch, marks a patrol turn, shifts weight, keeps a practical grip, and sometimes checks an assigned lane. Importantly, not everyone walks forever. *Custody holds a post*, while prison and yard sentries follow different purposeful routes. Footsteps, kit, fabric and breathing should depend on actual ground speed and surface. Final authored performances are not yet present.

**Suspicious.** The first clue: stop, turn head before shoulders, perhaps 'Huh?', then orient toward an identified stimulus. The player can still withdraw. There is no automatic facility-wide alarm from a single guard becoming curious.

**Investigating.** Move toward the *actual reported sound or body*; examine a plausible point, alternate view angle and search nearby. Repeated concrete impacts renew attention. A body is more serious than a single minor noise. The guard does not track Snake through the wall.

**Confirming.** Partial or sustained sight of an intruder, with the weapon not instantly dealing damage. A role matters: a custody officer can issue an actual stop/challenge and accept nonthreatening compliance after a short, visible interval. A hostile soldier must still confirm a target to attack.

**Engaged.** Real sight plus high numeric awareness: face/shoulders square up, the soldier may transmit a last-confirmed sighting, pursue or warn. For close-range melee, visual confirmation for .16s and facing alignment lead to a .38s telegraphed strike. If Snake reaches cover, leaves range or staggers the guard, it cancels. Those numbers are **tuning hypotheses**.

**Searching.** The enemy lost proof of current position. The soldier travels to last *evidence*—a sighting, sound or body—not a dynamically tracked player transform. A squad report arrives delayed; the receiver must decide how to investigate without omniscience.

**Returning.** After finite negative evidence, the guard recovers, checks surroundings and returns to the original duty route. Y-axis guards must not 'return' to an X-axis path they were never assigned.

**Incapacitated.** The victim remains an actual physical clue, not deleted from the simulation. Another guard must have distance and line-of-sight to discover the body. It can trigger a Caution report, not a magically confirmed new player position. Final knockout/death presentation and persistent save are blockers.

### Facility-wide state transitions

- **Normal → Caution:** credible suspicious investigation or reported body. Increased scanning and attention; routine custody procedures remain available unless full Alert/Evasion is active.
- **Caution/Normal → Alert:** actual sighting reported over radio after a finite delay. Local suspicion without radio does **not** automatically alert every sector.
- **Alert → Evasion:** the report's urgency expires after a bounded window. Other guards can investigate the *last report*, but remain subject to physical sensory evidence.
- **Evasion → Caution:** coordinated search pressure falls without renewed direct contact; the installation remains watchful.
- **Caution → StandDown → Normal:** finite return to routine rather than an instantaneous reset. Any new verified report interrupts this decline.

The source currently models approximate alert/evasion/caution/stand-down timers as **10s/7s/5s/3s**, with body Caution about 9s. Audio/VO could respond to the phase through Blueprint events; the events **do not** authorise extra enemy spawns, magic GPS updates, free damage or closed mission doors.

### Six real-world consequences of these distinctions

1. **The starting yard** has ordinary patrol duties. Snake can watch and learn routes without setting off a base-wide response.
2. **A POW clue** can prompt an intentional nonlethal capture plan. Security sees an unexplained intruder; it does not know Snake wants arrest.
3. **Damaging the cell walls** alerts guards by sound location and occlusion, potentially raising local suspicion.
4. **Rescuing Gray Fox** has no invisible 'spawn guards now' condition. The existing prison guards observe, investigate or react based on real evidence.
5. **A direct sighting** can lead to a report and facility Alert; escaping to cover transitions pursuit into search, rather than allowing through-wall attacks.
6. **A discovered casualty** changes the work of the remaining guards, with evidence, hesitation and communication, without pretending they've seen the attacker.

### What still needs authored animation and sound

Each meaningful phase should have a physical anticipation, readable action and recovery. A visible *report* posture needs a radio-hand animation, actual recorded bark and timed delay. A visible *challenge* needs an arm or weapon lowering and intelligible caption. A *strike* needs contact-frame animation, directional audio and an interruption window. Search requires head/gaze scanning and transitions based on evidence, not canned looping walk cycles. Each state has a Blueprint event in source, but **the actual cinematic-quality rigs, motion clips and voices have not been acquired or imported**.

### Test the combinations, not just individual states

Use a matrix: duty (3) × awareness (8) × evidence (8) × facility (5). Not every combination is reachable or desirable—**that's exactly what tests should assert**. The following should be impossible: a routine guard declaring a perfect hostile position from a body report; a custody officer accepting routine surrender during active lockdown; a dead guard launching a new radio call; an attacking guard completing a melee strike after losing sight; an off-axis sentry resuming the wrong patrol; or a published cinematic changing alert independently of gameplay evidence.

**Gates before 'production AI':** compile on the right UE5.7 toolchain, playtest each combination with no audio and no debug colours, simulate frame-rate spikes, test occluding doors/walls, check guard navigation around arbitrary geometry, and compare state logs against recorded player captures. The present implementation is a playable-*source* foundation, not proof of those outcomes.

See [machine-readable guard AI contract](../data/guard_ai_behavior_contract.json) and [security-state regression source checks](../tools/test_guard_state_layers.py).

## 19 November addendum — player-facing dialogue and actual facility report

The C++ guard now emits short **context-bound, non-global caption cues** for specific changes: “What was that?” after sound suspicion; “Check that noise” on investigation; “Intruder!” on confirmed combat sight; “Lost visual” after contact; “Man down!” on real body discovery; “Halt, keep your hands visible” for an actual custody challenge; and a radio report after its 0.6-second communication delay. The HUD displays the nearest guard's current callout **only within 950cm**. No VO or MGS trademark audio is imported or implied; `OnGuardCallout(FName)` provides the future sound/animation link.

The local report now snapshots the **last confirmed visual position** when a radio call begins. A later footstep or body report cannot quietly replace that payload. An actor's temporary suspicion does not by itself trigger global Alert. An actual transmitted sighting changes the facility state; the implementation gives all guards a slight, bounded vigilance increase while the installation remains on Alert/Evasion. Custody officers will not accept an ordinary nonthreatening detention during that lockdown, but guards still need individual evidence before pursuing or striking Snake.

**Temporary/prototype numbers:** facility Alert 10s, Evasion 7s, short Caution 5s, StandDown 3s. They control source state only; no validated scripted waves, bespoke VO, dynamic score or visual animations exist. Security-mode timers are not yet included in checkpoint persistence. The original MSX2 and the supplied J2ME edition are not used as proof of these *new* modern AI numerical parameters.

### State-leak fix

The mission's facility state **no longer** polls every individual guard's private Suspicious/Investigating phase to set global Caution. A guard may notice a noise while all other sections remain on their normal schedule. Facility Caution comes from an **actual reported body**, the residue of a previously transmitted hostile contact or the intentionally planted enemy transmitter. The local guard can still pursue its own physical evidence and the nearby HUD can show its caution. This prevents an unreported sound in the western yard from changing security vigilance in a far eastern room.
