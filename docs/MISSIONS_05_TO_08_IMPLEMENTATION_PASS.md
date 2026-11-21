# Source-first implementation pass — Missions 05 through 08 (21 November 2026)

**Status:** Source committed in `design/mission-01-spatial-encounters`. C++ builders, interactions, saves and a 10-test static-source contract exist. **Unreal 5.7 has not been compiled, PIE-tested, cooked or played.** The Connect2Media v3.3.0 a1 exact map has not been traced; positions and fight parameters are **provisional remake blockouts**, not claims of original room accuracy.

## What this pass actually changes

Existing source had declared `BuildDesertAndCowardDuck`, `BuildDeepBunker`, and `BuildTX55Finale` but no implementation, and never invoked any of them. Mission 5 had no connection to Mission 6. Mission 4 required a Card 7 story flag that actual Card 7 pickup never recorded.

The added `MGLaterMissionFinale.cpp` provides physical, interactive Mission 6–8 arenas. `MGLaterMission.cpp` now constructs them and routes interactions/ticks, and the equipment save retains owned weapons/items, including the oxygen tank, compass, Card 7 and Card 8. A real research service exit follows Madnar's revelation, rather than ending the player's traversal on a subtitle.

### Mission 5 — The Price of Metal Gear
- Madnar refuses until Snake has freed Ellen in Building One's basement.
- Player opens a real weak wall with an earned explosive, rescues Ellen and physically returns.
- After Madnar provides TX-55 vulnerability, the actual service exit unlocks Mission 6.
- Research laboratory/B1 return geometry remains a prototype. **No claim of accurate J2ME subterranean topology.**

### Mission 6 — The Cost of Loyalty
- Provisional 15,500 cm-long exterior with covered desert, a Card 7 gate, guard routines attached to real checkpoints and a hazardous water intake requiring the previously earned oxygen tank.
- A misleading truck order is *optional investigative evidence*, not an unavoidable cutscene that moves Snake by teleport.
- Coward Duck/Dirty Duck uses three visible prisoners as shields, including Jennifer's brother. The boss blocks handgun damage when the actual shot path passes close to any captive, so flanking matters. Hostage casualty/failure ratings are **not yet implemented**.
- After the boss falls the player must physically rescue *all three*, acquire the Card 8 pickup and remember the brother's **left escape ladder** clue before the next mission checkpoint.
- Credited equipment must be used in actual equipped state; nothing is teleported into Snake's inventory to make a boss solvable.

### Mission 7 — Orders and Evidence
- Card 8 checkpoint, a witness who identifies Big Boss, and a separate relay interaction for Snake to hear and then knowingly reject the abort order. No future Venom Snake revelation leaks into 1995.
- Player may override the final shutter through the guarded security terminal **or** spend plastic explosive at the maintenance bypass: both routes resolve the same persistent gate.
- Only after witness, disobedience and actual security breach can Snake access the TX-55 chamber.

### Mission 8 — Outer Heaven
- TX-55 is a **stationary target**, defended by cameras and two physically distinct foot targets.
- Place the original-comparative **16 plastic charges** in order **R R L R L L R L L R R L R L R R**. Each correct placement consumes an actual item and saves progress. Wrong-side attempts tell the player no charge was used.
- TX-55 destruction marks the event and starts **self-destruct first**, before the separate rocket-vulnerable final commander encounter.
- The reactor access gate opens after the sixthteenth charge and the commander cannot block the TX-55 demolition sequence.
- A saved, real countdown advances only in the active mission. On expiry Snake is killed and the existing Game Over / Continue screen handles recovery.
- Two ladders exist. The **right** ladder is an intentional decoy; the **left** ladder completes the mission only after the earlier brother rescue and final boss are resolved.
- No false 1995 Snake knowledge about the later MGSV Phantom retcon.

## Integration and QA

Files edited:
- `unreal/MGRemaster/Source/MGRemaster/Private/MGLaterMission.cpp`
- `unreal/MGRemaster/Source/MGRemaster/Private/MGLaterMissionFinale.cpp` (new)
- `unreal/MGRemaster/Source/MGRemaster/Public/MGLaterMission.h`
- `unreal/MGRemaster/Source/MGRemaster/Private/MGCampaignState.cpp`
- `unreal/MGRemaster/Source/MGRemaster/Private/MGEquipmentComponent.cpp`
- `unreal/MGRemaster/Source/MGRemaster/Private/MGSetpieceBoss.cpp`
- `data/1995_numbered_missions.json`
- `tools/test_missions_05_to_08_contract.py` (new)

29 independent, in-session **textual source consistency assertions** passed for builder presence, stage handoff, requirement flags, source TX-55 order, source chronology, rescue and equipment state, and balanced braces. These are **not C++ compiler tests**. The 10 source regression unittests have been committed but could not be executed against a local Unreal checkout from this environment.

**Required UE playability test:** compile in UE 5.7, spawn at each mission index, traverse all physical doors, verify no missing floors or impassable bar geometry, trace bullets/projectiles and hostage sightlines, test each weapon depletion case, test dying/Continue and reloading at all major milestones, and review the audio director, camera, Game Over and AI behaviour. Correct layout against J2ME MSX2 source reference before marking lore-accurate.

## Browser release

A separate **Gray Fox browser vertical slice**, not the eight-mission Unreal runtime, now has the timed left/right/kick combo and corrected Game Over Continue/Exit flow. Its downloaded bundle includes 14 original music WAVs and 46 original SFX WAVs. Node tests pass: campaign, rigs, adaptive audio, melee/Continue callbacks. Real browser WebGL/audio listening still requires a tester.

**No claim of full browser/Unreal parity** until the later eight-mission geometry and gameplay are ported to the browser as well.
