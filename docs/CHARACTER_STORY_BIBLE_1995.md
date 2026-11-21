# 1995 Metal Gear — character stories and encounter-beat bible

**Status:** full-campaign narrative planning registry, not fully implemented dialogue/cinematics. Companion data: [character_arc_registry.json](../data/character_arc_registry.json). Designed under Doctrines 000, 002, 003, 004, 005. Source priority: the supplied **Connect2Media J2ME v3.3.0** target (undecoded) > original **1987 MSX2** story/manual for comparison > subsequent game canon for restrained hindsight > clearly identified remake elaboration. Do not conflate narrative order with final floor layout until J2ME rooms/events are verified.

## Central emotional structure

This is **Snake's first test of independent judgment**, not merely a succession of boss fights. The player's changing relationship to trusted voices carries the drama:

1. **Dependence:** Big Boss's advice seems indispensable. Other radio voices feel risky and unfamiliar.
2. **Trust earned through action:** Snake finds Gray Fox, and an elite operative who was previously only a mission objective becomes a vulnerable human source of crucial truth.
3. **People before hardware:** Madnar's cooperation depends on Ellen's safety; Jennifer's own family is endangered. The player learns the facility houses people, not merely targets and keycards.
4. **Authority frays:** contradictory instructions, resistance intelligence and Big Boss's increasingly hostile orders force the player to evaluate evidence.
5. **Independence:** destroying TX-55 and confronting the leader resolves the same question introduced by the opening radio call: whose judgment does Snake obey?

The theme is **earned trust**. Character details must change a decision, reveal the place differently, offer a tool, or affect a real consequence. No lore-for-lore's-sake cutscenes.

## Act-by-act ensemble placement (provisional geography)

| Stage | Playable geography | Personal story emphasis | Required interaction |
|---|---|---|---|
| S01: insertion | Building One opening and reconnaissance | Snake trusts Big Boss; Gray Fox is missing; Schneider/Diane become intelligible alternatives | Early useful radio, guard-informed route, prisoner clue |
| S02: capture and rescue | Secret cells, equipment recovery and Shoot Gunner escape | Gray Fox moves from abstract target to resilient captive ally; Snake becomes rescuer | Physical capture, breach, rescue, real intelligence, uninterrupted escape |
| S03: scientist trail | Courtyard and Building Two approach | Resistance help becomes operationally tangible; elite enemies embody Outer Heaven | Machinegun Kid/Fire Trooper, Jennifer's assistance, actual route information |
| S04: hostages | Madnar cell, Ellen location and return path | Madnar's apparent obstruction resolves into parental fear; Ellen has a voice | Rescue Ellen, hear testimony, return and earn TX-55 weakness |
| S05: betrayal | Building Three, inner defences, Coward Duck | Jennifer's brother is endangered; Schneider's warning is cut short; Big Boss turns obstructive | Hostage-safe boss resolution, radio conflict, final sabotage planning |
| S06: resolution | TX-55, final confrontation, escape | Snake rejects compromised authority; Diane/Jennifer retain human connection | Player-controlled resolution, final calls, escape without expository retcon |

The stages express story order and dependency, **not authoritative mission numbering**. Physical paths and backtracking will be assigned to source-confirmed rooms; a single stage can revisit multiple buildings. Story overlays and optional conversations must never force meaningless waiting.

## Gray Fox: complete 1995 treatment

**Pre-arrival:** Gray Fox is FOXHOUND's most accomplished operative; his loss is a meaningful warning about Outer Heaven and about Snake's vulnerability. Use a pre-mission dossier photograph/briefing and a restrained radio mention, but do **not** show a fabricated canonical capture sequence or turn the missing man into a playable extra mission without source review.

**Finding him:** Evidence and fellow captives steer Snake toward capture and the secret prison. Schneider can optionally explain built alterations; the player must still physically discover/breach the wall. Gray Fox's cell entrance must be visible from believable architecture, not materialise behind the player. No requirement to listen to the radio to breach.

**Encounter:** A vulnerable but highly alert Gray Fox, not a superhero cameo. His first priority is a concise, crucial piece of intelligence: the nuclear-armed Metal Gear threat and the need to find its designer, Pettrovich. Camera, posture and subtitles should convey relief and authority without robbing Snake of control. Story commitment happens at interaction, independently of animation playback. Distinctive character silhouette/rig and actual VO are **missing**.

**After:** His rescue is a definitive story milestone but the 1987 game does **not** supply a long follow-on personal campaign for him. Resolve his local status through a durable intel log/optional short parting exchange. Avoid inventing a second playable Gray Fox boss, a long escort, a role in the TX-55 fight or a 1995 cyborg transformation. His later life is powerful *hindsight* from MG2/MGS, not a revealed memory for Snake here.

**Relationship:** He is a professional predecessor, not an instantly close friend. Snake's earned competence is what gains Fox's respect. An additional exchange of character, if added, must be optional and tagged **remake-only**, with no later-canon spoilers.

**Failure/skip rules:** Rescue cannot repeat; no player can leave the critical intelligence irretrievable; the prison walls cannot close on reload; guards react only to real events; the rescue event has no compulsory cinematic stall. Trigger IDs in registry: `fox.missing`, `fox.prison_clues`, `fox.revelation`, `fox.parting` plus Snake's `snake.rescue_fox`.

## Big Boss: advice must precede betrayal

Big Boss first provides **correct, useful** logistical and survival advice. Otherwise the player will recognise a trick immediately. Only later, after Snake has demonstrated unexpected progress, should information become suspicious. The original script includes a direct abort order, a later public revelation and final confrontation. Preserve the key deception's late-game position. Environmental evidence should permit attentive players to infer the betrayal early, but Snake must not assert a later-game twist he couldn't know in 1995.

Later-canon *MGSV* reframes the final opponent; that is a **retrospective/postgame continuity consideration**, never a mandatory 1995 exposition sequence.

## Pettrovich and Ellen: one story, two people

- The engineer is detained and coerced, and his daughter is used as leverage. When found, he refuses to give Metal Gear's weakness until Ellen is saved. The refusal must be framed as understandable fear, not arbitrary withholding or a random extra keycard requirement.
- Ellen is a hostage and former ballet performer, not an invisible state flag. Give her visible signs of life, a distinct performance and a short testimony about coercion. Her safety must create an observable change in the scientist's behaviour.
- The gameplay dependency is `madnar.condition` → `ellen.rescue` → `madnar.weakness`. Rescue state persists if radio or a cinematic is skipped. Provide a reasonable return route to the doctor rather than teleporting the answer into the HUD.
- Avoid claiming Madnar's later MG2 actions are already his 1995 intent.

## Resistance ensemble

**Kyle Schneider:** former fortress architect and a resistance leader after personal tragedy. His utility is uniquely architectural: alternative access, equipment rooms, blast walls and lift connections. Give his intel practical environmental confirmation. His near-final revelation is interrupted; **do not invent a confirmed death** or transplant his later sequel identity into the player-facing 1995 narrative.

**Diane:** former Thin Wall singer and resistance operative, expert in boss tendencies. Let her correct an enemy-specific misconception or provide a useful, optional tactical hint; she shouldn't run an all-knowing combat tutorial. Her final emotionally charged contact should be human, not a required romance.

**Jennifer:** undercover medical employee primarily motivated by saving her brother. Her help must cost something *to her*: the risk of exposing an insider, or using a real access route to deliver an item. Her important boss encounter is **Coward Duck** because he holds captives including her brother. The player needs a readable, nonlethal/careful-targeting option; harm to hostages is a tangible failure. Her brother can provide escape intel when saved.

**The wider prisoners:** they are useful sources of geography and narrative pressure, not interchangeable collectible counters. Each critical prisoner rescue should be placed in a physical cell, with a stateful identity and a reason Snake learns something. Keep generic 3F POWs separate from named Gray Fox, Madnar, Ellen and Jennifer's brother. Existing provisional `3F_PrisonerContact` must *not* be interpreted as Gray Fox.

## Boss characterization without false biographies

**Shoot Gunner:** enclosure/escape authority. A slow, heavy shotgun, clearly telegraphed firing commitment, strong angle control and room details suggesting detention enforcement. A concise threatening introduction is source-comparative; a detailed childhood/backstory would be remake fiction.

**Machinegun Kid:** suppressive corridor opponent. Rather than just more health, reward flanking and directed remote-weapon counterplay.

**Fire Trooper:** area denial. Flames and hazard geometry should force route choices that remain physically navigable and intelligible.

**Coward Duck:** moral pressure, not merely combat pressure. Human shields transform aiming, stealth and equipment selection into a rescue problem. Source-preserved identity, named captives and keycard rewards matter more than invented psychological trauma.

**Other enemies:** vehicles, Bloody Brad/Arnold androids, TX-55 and final Big Boss are important encounters but not automatically developed *personal* stories. Differentiate combat behaviour, world duty and production requirements in encounter contracts. A named boss gets a personal arc only if source evidence or an explicitly approved remake addition supports it.

## Narrative implementation requirements

Each beat in the registry is a proposal until implemented and playtested. For implementation, attach:
- a **stable event ID** and mission flags; prerequisites that prevent contradictory state but **never require an optional call** to continue;
- a world anchor (area ID or mapped entrance), interaction trigger, emotional intent, independent audio/visual cue, and an always-recoverable intel transcript;
- a clear source grade for dialogue and claims: `msx2_comparative`, `j2me_verified`, `later_canon_hindsight`, or `remake_optional_proposal`;
- skip/reload logic, minimum text legibility and no fabricated guard alert from non-diegetic radio;
- if voice and facial animation assets are absent, `design_only` or `prototype`, never `complete`.

`depends_on` in the narrative registry is **ordering/context**, not a directly wired hard game gate. The event layer must separate required source-state prerequisites from optional exposition, so missing radio calls cannot soft-lock the player. Map anchors such as `building_2.prison` are **planning identifiers** until matched to `area_contracts.json` or a source-confirmed new area.

## Fidelity and continuity review

- Primary original reference: https://www.konami.com/mg/archive/mg/chara.html
- Comparative 1987 script: https://gamefaqs.gamespot.com/msx/578853-metal-gear/faqs/30618
- Later Gray Fox reference: https://www.konami.com/mg/archive/mg2/chara.html
- Remake continuity policy: [CONTINUITY_AND_LORE_ENRICHMENT.md](CONTINUITY_AND_LORE_ENRICHMENT.md)

Current implementation is limited to **opening mission + optional radios + an isolated prison/Gray Fox rescue prototype**. Source-only actor and event machinery exists, but the complete campaign arcs, performances, named prisoner rescue chain, boss behavior and ending are **not** implemented. This document is planning evidence, not proof of a complete game.
