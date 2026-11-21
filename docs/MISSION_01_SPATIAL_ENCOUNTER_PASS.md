# Mission 01 — source-first spatial encounters (revision 2)

**Production status: design contract, NOT a playable level.** This supersedes the first speculative layout and must be read alongside `data/mission_01_spatial_blueprint.json`. Current area rectangles and guard waypoint coordinates are placeholders for remaster collision/blockout only. They are not reconstructed J2ME geometry or militarily prescribed patrol locations.

## Authoritative source hierarchy
1. Supplied Connect2Media J2ME 3.3.0 game and its actual room/event/item data — target game, decoding still incomplete.
2. Original 1987 MSX2 Metal Gear manuals and gameplay — historical comparison, not presumed interchangeable with the J2ME edition.
3. Community MSX2 floor maps and walkthroughs — explicit cross-checking aids, not primary proof.
4. Modern military public doctrine — reference for believable job assignments and behavior, **not** a mandate to replace canonical enemy choreography.
5. New transition architecture — clearly labelled remake-only, introduced solely for spatial continuity, camera clearance, stealth affordances, or comprehensible navigation.

Official story: Snake enters alone with a transceiver, searching for Gray Fox and ultimately Metal Gear. Do not insert starting combat equipment. https://metalgear.konami.net/manual/mc1/mg1_msx/pc/en/page02.html

## Canonical-comparative opening chain (MSX2, J2ME verification outstanding)

1. Initial approach: proceed past guards and crates, preserving the opening's observation and infiltration emphasis.
2. First three-truck area: **west = ration; centre = Card 1 after its guard emerges; east = binoculars**. The guard's exit/inspection is a memorable source beat. Make that visible and interruptible; never simply hand Card 1 to the player.
3. Card 1 room: opening requires Card 1, resting/sleeping guard and Gas Mask pickup. Keep the room physically navigable with 3D camera.
4. Second truck and elevator vicinity: separate later **handgun truck** (no starting ammo), **guard-heavy truck**, and **land-mine truck**. Guarded elevator is a major vertical circulation anchor. Do not collapse both truck clusters into one invented supply depot.
5. Building One upper floors: cameras and ammo on the third floor (and POW, hazardous-room progression) must be designed as a continuation, not replaced by an invented Mission 1 completion checkpoint.

Cross-check sources:
- https://thesnakesoup.org/metal-gear-msx-walkthrough/
- https://www.vgmaps.com/Atlas/MSX/
- https://gamefaqs.gamespot.com/msx/578853-metal-gear/faqs/7524

## Guard placement doctrine for a Metal Gear game

Public references: US Army *TC 3-22.6 Guard Duty* (2017), and *ATTP 3-39.32 Physical Security*. These describe guard duty, assigned posts, patrol and communication responsibilities. They **do not** validate exact fictional coordinates and should never be used to claim a real base-security plan. Interpret them for atmospheric credibility:

- Assign each actor a comprehensible duty: crate-route observation (G01), truck-yard watch (G02), original centre-truck departure beat (G03), later vehicle-route inspection (G04), elevator access duty (G05), optional maintenance watch (G06).
- Give each a believable routine: checking a truck, monitoring an access point, inspecting damaged equipment, pausing to communicate. Avoid identical looping robots unless a source pattern requires it.
- Use coherent guard states with diegetic tells: idle duty, observation, mild suspicion, investigate, confirmed alert, localized search, resume/report. A guard knowing a suspicious noise's source is **not** the same as knowing Snake's position.
- Communication is believable and imperfect: a sighting should matter, while an unverified sound must not instantly inform every soldier.
- Preserve source-specific moments even when they look less than tactically optimal. Make original routines clear enough for stealth timing without forcing a wait or a fight.
- Protect player enjoyment: offer an observation position before threats, legible guard intentions and more than one escape/recovery choice. Optimize for fairness rather than creating an unbeatably efficient security system.
- No fabricated spawn waves, alarm resets, or guards appearing through solid geometry. Avoid tracking the player through walls without sensor evidence.

Public historical doctrine reference is especially useful for period-inspired posture but the modern texts are **not proof of Outer Heaven's own 1995 orders**. No invented radio channels or squad deployments are asserted as canon.

## Architecture policy

The currently proposed drainage and maintenance connectors are **optional remake-only candidates**. The default source-fidelity route must remain viable without them; no optional transition may bypass required original item, boss, rescue or elevator gates. Their exact existence should be revisited after the J2ME map trace.

A true three-dimensional adaptation must represent vehicle bays, truck interiors, guard stations, functional loading doors, stair and lift shafts, secure rooms, backtracking corridors, ceiling clearance, and usable cover. Transition rooms exist to explain buildings physically, not to increase walking time. Distances and angles must be tuned by first-playthrough metrics and verified geometry rather than invented original dimensions.

## Required test routes

- First-run no-combat arrival → first truck yard → collect Card 1 → Gas Mask room → second truck yard → lift.
- Expert route preserving original progression but minimizing unnecessary stops.
- Alarm triggered at either truck yard: no unavoidable checkpoint trap, guards investigate plausible locations, no arbitrary reinforcements.
- First truck departure guard interrupted, distracted and allowed to complete routine; Card 1 never despawns or duplicates.
- Handgun pickup grants gun with no automatic ammo. No ammo soft-lock on the elevator route.
- Both shoulder cameras clear truck doors and corridor turns, including prone/crouch and alert retreat.
- Door/card, lift, item, guard awareness and opened connectors persist through save/load and revisit.
- Side transitions removed: full critical route and source items remain reachable.

## Execution status and next work

Spatial JSON and static coordinate validation are committed on a draft PR. Build-time importers, functional guard AI, source geometry reconciliation, browser/Unreal content spawning, weapon pickup inventory and UE compilation **are not yet done**. Do not claim that a document or a coordinate graph is playable.

Source-based annotation should next begin at the first-truck area and extend through the upper-floor security/camera layout. Each source screen needs an ID, screenshot reference, original actors/items/events, adjacency, 3D transition treatment and fidelity decision before converting it to production geometry.
