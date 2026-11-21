# Solid Snake Is Not Naked Snake — 1995 Playable Character Doctrine

**Status:** Binding character-direction guidance. Implemented features and outstanding work must be separately tracked. This is *not* a declaration that the current Unreal project has a finished character rig, animation blueprint, facial performance, or validated playability.

**Short version:** Naked Snake's *MGS3* fantasy centres a field operative mastering an ecosystem; Solid Snake's 1995 *Metal Gear* fantasy centres an initially unproven operative learning to turn an enemy's military facility and intelligence network against itself. Both have combat skill, tactical intelligence and empathy. We distinguish **gameplay emphasis**, not superhuman biology, capability ceilings or crude good-versus-bad morality.

**Primary machine-readable contract:** [solid_snake_1995_playstyle_contract.json](../data/solid_snake_1995_playstyle_contract.json).

## First rule: a rookie is not incompetent

Konami's original Solid Snake character profile calls him a freshly recruited FOXHOUND rookie, with Operation Intrude N313 his first assignment. Konami's manual gives him little starting equipment and requires the player to find weapons, keys and information and rescue captives. His **player controls must still be precise immediately**. Inexperience belongs in decisions under pressure, calls that reveal limited knowledge, and what the player is forced to learn—not input delays, intentionally incompetent firearms or unusable melee.

Source: https://www.konami.com/mg/archive/mg/chara.html
https://metalgear.konami.net/manual/mc1/mg1_msx/pc/en/page02.html
https://metalgear.konami.net/manual/mc1/mg1_msx/pc/en/page08.html

## Eight playable traits

1. **Brilliant observer and opportunistic improvisor.** The player notices guard posts, floor layouts, cameras, machinery, equipment and prisoner clues; earlier observations become advantages on returns. A missile should hit a real switch, a gas mask should protect from real gas and a discovered mine cache should make later tank preparation intelligible. A radio contact can direct attention but should not solve a puzzle automatically.
2. **Quietly relentless.** Snake tries another plan after a setback. His strength is measured pressure, not loud invincibility. Konami's later retrospective notes his emotional understatement after the Outer Heaven mission. The later older-Snake mood is a continuity guide, not permission to give the rookie the entire future veteran's personality.
3. **Humanity demonstrated by action.** POW and hostage rescues carry story, rank, intelligence and downstream consequences. Saving Ellen before returning to Madnar, and safeguarding Jennifer's brother before his escape advice, must feel like protecting people rather than buying an unlock token.
4. **Learned skepticism.** Start with useful Big Boss guidance. Let Snake notice contradictions with the fortress and survivors; gradually increase doubts. The player **chooses not to comply** with compromised instructions once evidence is sufficient. Do not telegraph the twist in the opening title art or camera language.
5. **Efficient, reactive physical combat.** Solid can use CQC; do *not* reduce him to a man who cannot grapple just because Naked Snake's MGS3 game foregrounded CQC. But favour simple, readable, purposeful restraint, quick disengagement, weapon control and awareness of sightlines over needlessly elaborate combo strings.
6. **Dry and pointed conversational voice.** Questions should request clarification, detect an evasion, or reveal the absurdity of a dangerous order. Repeating an unfamiliar term can be a character signature once in a while, but not a substitute for witty writing and active intelligence.
7. **Precise body/space awareness.** Wall cover means his **back faces the surface**, shoulders align with its normal, feet sidestep and head turns toward a corner. A rear choke has proper approach angle and struggle, and a body must be dragged at reduced speed with weight and contact through navigable space. These are modern-MGS interpretations layered over the 1987 loop.
8. **Distinct from his mentor, not beyond him.** Big Boss and Snake share training. Their divergence is expressed through *choices*: a commander uses people as a means to preserve Outer Heaven, while Snake's actions continually disrupt that purpose. The full father/clone/phantom lore is future context and is not introduced in 1995 dialogue.

## How to direct the camera, body and player control

**Movement:** immediate acceleration, predictable deceleration, responsive crouch and recoverable transitions. Use the environment to create pressure rather than sluggish animation locking. Running should be audibly risky when patrols are nearby; quiet walking, corner peeking, low posture and an earned rear takedown are different choices.

**Wall cover:** capsule normal, shoulder axis and animation root must agree. Keep Snake back-to-wall and facing along the wall, never nose-to-concrete. Correct the cover aim offset, no instant 180-degree animation snaps, and permit disengagement before the environment makes the player vulnerable.

**Guard approach:** display the choke prompt only in rear cone and reach, provided the guard is actually unaware. A live restraint has motion and potential failure; an unconscious guard stays present. Dragging becomes a slow, physically plausible action, not an invisible item pickup. Hiding a body requires a real environmental object, such as an appropriate service locker.

**Gear:** equipment belongs in actual source-comparable storage areas: trucks, secured rooms, logistics stations, prisoner-assisted access, later weapon cages. The handgun, mines, guided missiles, gas mask, binoculars, explosives, grenades and rockets must create distinct tactical possibilities and boss preparation. No major weapon drops into Snake's inventory merely because the story demands a battle.

**Physical performances:** Snake's spine, shoulder line and hands match objects, props match their functional use, and animation syncs to actual collision and interaction state. This is not yet achieved by current greybox proxy capsules and raw code animation booleans. Require finished rigs and visual testing before marking done.

## Eight-mission dramatic expression

| Mission | Solid's distinguishing trait in action | What the player should learn |
| --- | --- | --- |
| 1 — Operation Intrude N313 | Curiosity and survival without preissued weapons | The enemy's facilities are a puzzle; competent stealth buys exploration time |
| 2 — The Missing Fox | Controlled self-sacrifice and improvisation | A skilled rookie can choose vulnerability to rescue someone more experienced |
| 3 — The Scientist Trail | Replayable intelligence and equipment payoffs | An earlier binocular, mask or card can change how a return route works |
| 4 — The Outer Defences | Conditional trust in Jennifer and tactical preparation | Helping people provides real assistance, not free ammunition out of thin air |
| 5 — The Price of Metal Gear | Prioritising Ellen over immediate technical gain | His ethics emerge through physical choices and backtracking |
| 6 — The Cost of Loyalty | Testing dangerous directions rather than blindly obeying | Snake can empirically disprove a commander's advice |
| 7 — Orders and Evidence | Deliberately acting on evidence and rejecting a false authority | Snake becomes his own leader |
| 8 — Outer Heaven | Calm sabotage, confrontation and remembered rescue payoff | The legendary outcome is *earned*, with an understated final report |

These are **proposed remake mission divisions** rather than official numbered chapters in the 1987 source. The actual J2ME 3.3.0 event/room map is still unverified; MG1 MSX2 is the comparative original.

## Deliberately do not import MGS3 systems by default

Naked Snake's *MGS3* design foregrounds camouflage scores, eating food to sustain a stamina gauge, medicinal cure and field treatment, and formal CQC. Those make sense for a jungle survival game. They do **not** automatically improve our indoor 1995 remake. Instead, create meaningful survival pressure from scarce ration pickups, persistent health, dynamic guard response, source-placed equipment and return navigation. Allow modern CQC / dragging / cover when it strengthens a faithful infiltration experience; mark those as *modernized gameplay additions*.

Konami *MGS3* design: https://www.konami.com/mg/archive/mgs3/uk/system_main.html
Konami *MGS4* CQC support: https://metalgear.konami.net/manual/mc2/mgs4/ps5/en/page14.html
Konami Solid Snake retrospective: https://img.konami.com/mg/mc2/s/img/top/book_sample_mgs4_en.pdf

## Production acceptance

- A player who doesn't know the series recognises Snake as smart **through the actions available to them**, not a voiced claim about IQ.
- A player who knows Naked Snake recognises shared training but a different loop: urban/industrial problem solving and relationships over food/camouflage management.
- Snake's first-mission status remains credible without ever deliberately degrading input responsiveness.
- Choke, carry, cover, environmental interactions and animation states correspond to geometry, a real action and an actual NPC consequence.
- Player actions to protect people change subsequent places, choices or encounters, not merely rank counters.
- The antagonist remains a useful commander before his lies become discoverable.
- All mission, radio, model and animation commitments carry a real build check before being labelled production-complete.

**Current implementation honesty:** Unreal source includes floor/mission prototypes, partial equipment discovery, radio/persistent story state, and experimental wall-cover/choke/drag/locker code. It does **not** include a finished performance-quality character mesh/rig or a compiled-and-tested implementation of the full contract.
