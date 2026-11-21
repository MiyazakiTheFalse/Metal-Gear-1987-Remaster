# 1995 campaign runtime bridge — Building One → Gray Fox → Building One

**Implementation date:** 21 November 2026

**Status:** Source committed to Unreal and Rust modules; **not compiled or playtested**. This integration is a step toward a connected game, not proof of source-exact level topology, finished cinematics, fully rigged actors or a deliverable end-user build.

## What is now a game event instead of only a story document

The campaign no longer always boots the same slice regardless of progression. A `UGameInstanceSubsystem` (`UMGCampaignState`) loads versioned `MG1995CampaignV1` SaveGame state. The chapter is one of:

| Chapter | Running actor | Entering | Leaving |
| --- | --- | --- | --- |
| 0 — Building One opening | `AMGMissionOne` | Fresh insertion on 1F; recover original-comparative equipment, use lift and clear Card 1 rooms | On 3F, rescue first prisoner, obtain detention transfer intel and **choose** to interact with the detention-transfer marker |
| 1 — Gray Fox prison | `AMGGrayFoxMission` | Fresh prisoner-cell spawn; capture action confiscates equipment | Break into Fox's cell, rescue him, breach the exit wall, **reclaim confiscated gear from corridor locker**, then physically reach corridor exit |
| 2 — Building One return | `AMGMissionOne` | Resume at saved upper-floor elevator landing with Fox's intel and same collected equipment/consumable counts | Scientist/Building Two journey **not implemented**; objective acknowledges next source-verified route |

The two provisional world actors never spawn together. The temporary transition is `UGameplayStatics::OpenLevel` selecting the correct actor on startup. This eliminates collision overlap but **is not the final physical basement link or seamless streaming solution**. We must replace it with properly authored elevators, stairs, prison entry, story acting and level streaming after tracing original J2ME rooms. A deliberate capture is **source-comparative**, not yet proven to be at our 3F coordinate.

## Story transactions and source authority

Permanent beat IDs are tied to actual interactions and the project-wide [1995 character arc registry](../data/character_arc_registry.json), including `snake.mission_start`, `fox.missing`, `snake.early_pow`, `snake.capture`, `snake.rescue_fox`, `fox.revelation`, `madnar.named`, `snake.gear_recovery`, and `fox.intel_persists`. Recording a beat commits text intel to a replayable campaign log. An unfinished audio cue or skipped transceiver line cannot erase the objective. UMGCampaignState rolls back pending in-memory changes if writing the save fails.

Schneider's and Big Boss's **opening** transceiver text is now stage-reactive and authored as provisional remake dialogue. The `J` mission log on Building One displays durable intel; the prison slice retains its own live dialogue/intel output. Before Fox rescue, Snake knows only that Fox is missing and Metal Gear is mysterious. Afterward, he knows its nuclear role and Dr. Pettrovich lead. Big Boss's betrayal and later-canon Venom identity are **not** revealed early.

## Actual recoverable equipment

The existing prison source previously could escape with equipment implicitly confiscated and then restore the entire original loadout after the return. That has been replaced with an interactive `ConfiscatedGearLocker` marker in the guarded corridor. It restores **only equipment originally collected** in Building One (P01–P07) and exact saved handgun rounds and ration count, including an empty handgun. Prison escape is blocked until locker state is committed.

`GrayFoxSliceV1` SaveGame schema is upgraded to v2 with `bGearRecovered` and migration from v1. After gear recovery, firing a handgun or consuming a ration persists the actual spent quantity to `MissionOneV1` v4, preventing free replenishment on the chapter transition. The old standalone test save is cleared *once* as the player first chooses the new campaign capture to prevent pre-completing a fresh rescue. On an interrupted post-escape reload, the source-side recovery path restores Fox's story flags and continues the transition.

## Controls and player-led timing

- Normal play: start on Building One 1F. Find Card 1 in the first trucks. Take the elevator to 3F. Open the Card 1 prisoner cell, free the prisoner and find the visible detention-transfer marker. **E/X** triggers capture deliberately, never automatically merely on approach.
- Prison: inspect/breach repaired walls, interact with Gray Fox, breach exit, recover your equipment from the visible corridor locker with **E/X**, then move to the escape checkpoint. The source rescue text is non-pausing; the info persists to the story log and new objective.
- Return: Snake resumes at the Building One upper-floor landing; saved pickups/door statuses, ammunition, rations and original story events remain committed. The objective points toward Dr. Pettrovich but does not pretend Building Two exists.
- **J**: show/hide recent mission intel on Building One. **C**: open transceiver, **Q** changes between Schneider and Big Boss; these are optional character exposition lines, never hard progression gates.
- **F9 developer only (non-shipping):** in Building One, clear the campaign and both mission test saves for a fresh run from the beginning; inside Gray Fox prison, restart the prison test checkpoint alone.

## Narrative/performance tasks still missing

1. **Exact J2ME map and events**: recover original source resource/tile and event traces; place capture and Gray Fox cells on a faithful Building One basement route, not a prototyping handoff at 3F.
2. **Greybox production blockers**: test line-of-sight from all critical pickups, guard paths and body placements; eliminate invalid shortcuts and clipped cameras; NavMesh and per-deck security volume correctness.
3. **Gray Fox/actor visuals**: realistic and recognizable fully rigged Solid Snake/Gray Fox/guards, prison vulnerability animations, actual body motion, face and voice, observed capture sequence, physically opened walls and functional supply locker art.
4. **Shoot Gunner**: source-constrained shotgun actor/boss arena, original equipment recovery beat, exit sequence and meaningful damage/health balancing. This remains absent, not implied complete by corridor guards.
5. **Full story**: other 100+ planning beats and later buildings, Madnar/Ellen return route, Jennifer, named bosses, TX-55, Big Boss betrayal and escape are still design.
6. **Browser parity**: Rust `campaign.rs` now supplies a tested-in-source pure transition policy; it is not linked into a browser runtime because the checked GitHub repository has no browser runtime source.
7. **Build validation**: no UE5.7 executable, compiler report, playthrough capture, Rust unit test execution or Python test run was produced in this environment.

## Acceptance and replay matrix

- Fresh boot starts at Building One, and the first radio/log does **not** reveal betrayal or Madnar's answer.
- Before rescuing the 3F POW, capture marker denies transfer. The prisoner must be reachable through the Card 1 doorway, not from the solid-wall exterior.
- Freeing POW saves clue once; repeat E does not duplicate intelligence. On reloading save, transfer stays available.
- Choosing transfer moves to Gray Fox standalone space and leaves no 1F/3F actors, patrols, CCTV or pickup collisions behind.
- Capture actually confiscates equipment; fire/heal attempts are denied.
- Rescuing Gray Fox commits Fox's nuclear warning and Madnar lead **before** subtitle timing ends.
- Attempt escape without locker recovery: **denied**, objective explicitly requests equipment. Find locker, restore precisely original inventory; collected ammunition only, no magically free gun.
- Reopen prison save after locker collection; locker remains gone and recovered gear remains available without collecting twice.
- Fire one round and consume one ration *after* locker recovery. Return to Building One; their amounts stay reduced.
- Escape returns to 3F/upper-floor lift, keeps all picked-up item locations empty and guards in their source-authored world. No extra mandatory radio listen.
- F9 from Building One intentionally clears all three development saves; next boot returns to mission opening with empty equipment.
- Repeat at 30/60/120 fps and with gamepad, disrupted save, death, skipped scene and no audio; record actual data rather than trusting source-only assertions.

## Regression coverage

- `tools/test_campaign_chapter_bridge.py`: chapter launch, POW capture, Fox intel and exit, locker/inventory, radio/HUD, Rust source parity.
- `tools/test_mission_01_runtime_contract.py` and `tools/test_building_one_elevator_contract.py`: updated to reflect the seventh pickup, two mission actors and Mission One save v4.
- `rust/src/campaign.rs`: portable, dependency-free transition logic and native unit tests.
- `data/1995_runtime_chapter_contract.json`: machine-readable accepted story states and incomplete areas.

**Validation performed at authoring:** 19 read-only source/contract checks passed. **Not performed:** Unreal compilation, Rust `cargo test`, Python `unittest` in a checked-out repository, or an actual game playthrough. Keep draft until those are completed.
