# Side-game influence doctrine — Outer Heaven 1995

**Source state: 20 November 2026.** This is a *method for building the complete 1987 MSX2 remake*, not permission to import any adjacent game's story. The [machine-readable matrix](../data/SIDE_GAME_INFLUENCE_ADAPTATION_MATRIX.json) is authoritative for the source/idea/status distinction. Supreme [Doctrine 000](DOCTRINE_000_HOW_WE_BUILD.md) and original MSX2 chronology always take precedence.

## 1. What to borrow, in order of value

**Metal Gear: Ghost Babel (GBC): meaningful logistics and isolated skill rehearsals.** The box-and-conveyor puzzle is memorable but can become tedious when progression depends on memorizing colours and roundtrips. Create a signature freight-bypass interaction and keep the box useful as recurring equipment once earned. Revisit logistics only when it creates a new, useful choice: a working conveyor, surveillance sweep, staff duties, local isolation switch, and alternate walking routes. Do not repeat an arbitrary colour-code maze every mission. A box should hide a patient, motionless Snake from remote guards, but near enough to examine an unlisted parcel is evidence of something wrong. There must be a *warning and time to respond*, not a guard's magical identification through cardboard.

**Metal Gear Solid: Peace Walker: optional briefings, not mission fragmentation.** Preserve a contiguous infiltration game. Let players request deeper factual context on their codec or in a nonblocking after-action review. Do not force cutscenes every 30 metres, introduce Mother Base building/factory management, or give Snake source facts he has not earned.

**Portable Ops: 1995 Gray Fox as a person, not an exposition dump.** Konami discusses Null and Big Boss's relationship in its retrospective. Franchise canon priorities surrounding that material vary. Gray Fox's 1987 mission is about imprisonment, exhaustion, observation and earned trust; show those through his timing, listening, scars and caution. Do not insert an unrequested Null flashback or pre-explain the 1987 ending.

**Metal Gear 2: Solid Snake: systemic stealth craft, not future history.** Use legible footstep/guard/cover interplay, vent/drain design with actual height and realistic world geography. The 1999 storyline and later-earned identity knowledge do not occur in 1995.

**Metal Gear Acid: puzzle combinatorics, not cards as powerups.** At each difficult crossing/boss, ask whether the player can use cover, timing, noise, position or equipment as an actual alternative with causal consequences. Never inject deck-building statistics or collectible attacks into the real-time campaign.

**MGSV: quality discipline, not an architectural source.** Animated stance changes, contact IK, weapon-hand alignment, quiet story beats, guards with duties, material cohesion, player feedback. Diamond Dogs Mother Base is not the canon blueprint for the fortress of Outer Heaven.

## 2. The important 1987 equipment exception

The 1987 MSX2 game places its permanent **cardboard box in Building 1 on the third floor behind a Card 2 door**. Independent MSX2 walkthroughs confirm the acquisition. **It remains a reusable, general-purpose stealth item after collection**: original MSX2 walkthroughs explicitly reuse it in later rooms and on B2 against guards/cameras. Certain specialist rocket guards see through it, so it should be reliable rather than magical. The browser/Unreal dispatch yard is a new 3D adaptation and must not steal that pickup.

**Current source prototype (not the campaign-wide box rule):** The opening offers a temporary *borrowed freight carton* (internal ID `FREIGHT_CARTON` in Unreal) for its working conveyor. It is lost at custody and is not returned as the permanent original item. The authentic `BOX` remains unearned until the Card 2 / B1-F3 route exists. **After collection, the permanent BOX must remain useful throughout the rest of the campaign, including later camera crossings and carefully chosen freight routes.** This is never a one-floor consumable. Browser has a separate, currently locked canonical slot; production art remains unverified.

Smoke from cigarettes near a freight optical beam is explicitly **optional new remake staging**, not a claim that the 1987 game featured smoke-revealed sensors. The *original* cigarettes' ability to help extend the late self-destruct escape remains a **future canonical implementation requirement**. Exact timer units and version differences require checking a primary MSX2 run before final balance.

## 3. Pacing: a moving break in the tension

The box must sometimes **reward clever stealth with temporary relief and real forward movement**, not add another puzzle. After a difficult alert or fight, Snake reaches a calm, working supply corridor with crates, tired staff and a real belt. When he hides inside a plausible consignment, the player can look around and hear ordinary conversations as the belt carries him toward a meaningful new area. Preserve camera control and an emergency opportunity to exit rather than forcing a long cutscene; leave a normal walking route too.

This works because apparent absurdity creates a short, welcome lull while still advancing the plot. Ordinary labelled freight should not be suspicious to every guard, and the next challenge should emerge after the player has enjoyed a little breathing room. Return to the equipment in distinct situations later, without multiplying tedious box-and-belt mazes. **Only the early freight segment exists in source currently; later passages are designs.**

## 4. Two-stage environmental suspicion

1. **Normal patrol:** box stationary, no alerted visual contact, and guard not within one body length with a direct front-facing look → no automatic identity proof.
2. **Curiosity:** stationary unlisted carton within ~1.65 metres, directly visible in a guard's forward sector, *not* a clearly legitimate parcel in the marked freight transit lane → guard speaks, raises cautious suspicion, and faces/approaches the real object. No combat privilege yet.
3. **Inspection:** continuous direct sight for about 1.3s → the cover's contents are confirmed; only now does the normal alert/aim process gain permission to identify Snake. Turning away or blocking LOS resets that close-inspection timer. An already alerted guard may pursue a moving carton normally.
4. **Counterplay:** wait for the guard to turn, move into actual cover, escape before proof, or choose an actual marked cargo route. Firing or moving conspicuously in the box remains unsafe.
5. **Presentation:** short original guard dialogue plus future hand-at-carton/lift performance via `OnCartonInspectionConfirmed` event. **No such final animation has been imported**; the event is a contract, not a finished cinematic.

Browser source: `browser/game.js`, `browser/dialogue.js`, `browser/tests/test_campaign.cjs`. Unreal source: `MGGuardActor.cpp/.h`, `MGPlayerCharacter.cpp`, `MGEquipmentComponent.cpp/.h`, `MGGrayFoxMission.cpp`.

## 5. Optional extras remain outside campaign

A Ghost Babel-style **VR/stealth drill** should be offered after mission completion or from the menu as a deliberately out-of-continuity practice room: no free ammo/rank/item in real saves; no alternate biography/dialogue; isolated checkpoint. The existing browser Shoot Gunner rehearsal illustrates this separation, but **a full skill-drill system in both engines does not yet exist**.

The same goes for Peace Walker-inspired mission debriefs: provide optional evidence summaries and stealth feedback without treating them as a new canon cutscene or modifying the default original ending.

## 6. Acceptance before production approval

- Authenticated source map/order for B1-F3 Card 2 and cardboard-box acquisition, then physical 3D rooms and elevator connections. The box is not automatically obtained at the opening. Once earned, the canonical box must survive sector changes and save/reload, remain available for repeated concealment, and never disappear after one conveyor ride.
- Test a deliberate post-danger breather: box rides past a believable work crew, player retains the camera, the route connects physically to the next objective, and a normal path stays available.
- Browser and UE source parity in borrowed-carton lifetime, suspicion and weapon/card gates; no mirror-world shortcuts.
- Confirm close inspection has a real occlusion check, visible action warning, sensible guard-facing and enough recovery time. Verify it under camera motion, crouch/prone and previous alerts.
- Save/reload pre-capture, mid-conveyor, after capture and after gear recovery; no equipment reappearance or duplicate ammo.
- AnimBP/rig, paper deformation, hand contact, foot placement, material/art/audio provenance and novice playtesting recorded before changing the status from `source_prototype` to `engine_tested`.
- Browser logic tests, Unreal compilation and low-spec gameplay capture separately. Do not confuse source static checks with a production-quality game.

### Reference sources

- Original MSX2 campaign: https://gamefaqs.gamespot.com/msx/578853-metal-gear/faqs/7524
- Konami MSX2 manual: https://metalgear.konami.net/manual/mc1/mg1_msx/pc/en/page10.html
- Nintendo Ghost Babel summary: https://www.nintendo.com/en-gb/Games/Game-Boy/Metal-Gear-Solid-Ghost-Babel-265999.html
- Ghost Babel design retrospective: https://nintendowire.com/features/taking-a-look-back-at-the-game-boy-colors-metal-gear-solid/
- Konami Portable Ops summary: https://eu-support.konami.com/hc/en-gb/articles/9822256083863-Backstory-What-is-the-story-of-Metal-Gear-Solid-Portable-Ops
- Peace Walker mission formats: https://www.gamespot.com/articles/metal-gear-solid-peace-walker-hands-on/1100-6229066/
