# Pacing Research, Revised Prototype Tunables, and Measurement Plan

**Status:** Research-informed design hypotheses, NOT empirically validated numbers. Governing policy: [Doctrine 000](DOCTRINE_000_HOW_WE_BUILD.md), [Doctrine 003](DOCTRINE_003_PACING.md), [Doctrine 004](DOCTRINE_004_INTEGRATED_EXPERIENCE.md), [Doctrine 005](DOCTRINE_005_SPATIAL_STRUCTURE.md).

## What the evidence actually supports

**Wauck & Fu (2017), A data-driven, multidimensional approach to hint design in video games:** spatial-puzzle experiments show that exploration can support player experience, and players can prefer fewer hints than some automatic/adaptive conditions. It does *not* prescribe a universal hint time; their sample and task were not Metal Gear. Source: https://experts.illinois.edu/en/publications/a-data-driven-multidimensional-approach-to-hint-design-in-video-g/

**Hao et al. (2022), An Adaptive Hint System for Puzzle Games:** motivation includes hints arriving at the wrong moment in a player's flow or revealing too much. This supports state-sensitive, escalating information, not any unique threshold. Source: https://xplorestaging.ieee.org/document/10017301

**The Level Design Book, Pacing and Composition:** room geometry, mission route and changing risk affect the player's tension curve. Spatial structure and readable affordances should be tuned before forcing audio/cinematic waits. Sources: https://book.leveldesignbook.com/process/preproduction/pacing and https://book.leveldesignbook.com/process/blockout/massing/composition

**GDC 2017 Level Design Workshop:** level designers describe considering both macro/micro pacing as well as spatial support for stealth (occlusion, multi-path, vantage points) and combat (cover, flanking, tight/open spaces). Source: https://www.gamedeveloper.com/design/gdc-2017-level-design-workshop

**Bringing Balance to Stealth AI in Splinter Cell: Blacklist:** fairness and transitions between patrol, investigation, search and alert demand geometry-aware AI knowledge. Sounds may propagate via chokepoints rather than straight-through-wall distance. Source: https://www.gamedeveloper.com/design/bringing-balance-to-stealth-ai-in-splinter-cell-blacklist

**These references explain why to adapt. None proves that 28s, 75s, 4.5s or 0.52s is optimal.** The following are deliberately revisable starting points.

## Implementation/timing decisions (2026-11-18, current source branch)

| Mechanic | Old source | Revised source candidate | Rationale and uncertainty |
|---|---|---|---|
| Initial optional hint | 14s since progress | 28s since progress AND 6s without significant movement/camera scan | Reduce premature hand-holding of an exploring player. Threshold not validated. |
| Escalated optional hint | 35s since progress | 75s since progress AND 10s without significant movement/camera scan | More time to generate and test an environmental hypothesis before stronger help. |
| Anti-stall assistance | None | 100s soft/150s strong caps regardless of continuing movement | Avoid an indefinitely stuck player receiving no help because they keep looking around. |
| In-world radio | Blocked before rescue / pause | Optional manual C/Start; Schneider 120.79 and Big Boss 120.85; no game pause | Let players request context at their own pace, preserve 1995-era presentation. |
| Guard turn/reaction | 0.22-0.45s | 0.31-0.52s | Provide a perceptible reaction and exploitable moment; highly dependent on guard animation/position and should vary with alertness. |
| Guard searching at sound | 2.5s + 0.85s per renewed knock | 4.5s + 1.25s per renewed knock | The prior search was short relative to taking advantage of a diversion; navigation may dominate actual investigation duration. |
| Gray Fox cue readability | 11s subtitle display, no input lock | 7s temporary subtitle display, no input lock, essential intel persisted | Avoid visual lingering while allowing free exploration; **final duration must be based on voiced delivery and reading tests**. |
| Wall knock cooldown | 0.7s | 0.7s (unchanged pending authored rig) | Avoid audio spam, but final spacing must respect hit/anticipation montage and hardware latency. |
| Input-to-observable response | No measured threshold | <=120-150ms *target* in beat contracts; not a measured or universal guarantee | Responsive feel requires scene-specific profiling. This is a budget to test, not a hard research limit. |

## Limitations of the current exploration proxy

The UE source tracks location and controller view changes between frames; it does not truly infer curiosity, conscious attention, whether a player is studying a repair seam, reading the radio or testing a hypothesis. Frame-to-frame low thresholds can be noisy. The 100/150-second caps ensure hints aren't suppressed forever. **Research suggests observing real player states; this implementation is an intentionally simple first-pass proxy.**

## Test with player cohorts; do not tune by opinion alone

1. Recruit first-time players and experienced stealth players; do not coach them through the cell. Record whether they identify the wall, how long until first voluntary inspection, total stuck time, clue acknowledgement and self-reported interruption annoyance.
2. Compare three variants: no automatic hints but always-available radio, current 28/75 adaptive proxy, and a more conservative 45/100 adaptive variant. Rotate order across participants and log source/level versions.
3. Test environmental wall cues with sound muted and at low resolution. Do not assume all players recognize cracks or hollow sounds.
4. Capture synchronized input, frame timings, guard state, voice/caption onset, physical wall change, and actual player control lock duration. Run speedrun, cautious, backtracking, save/reload and guard-detected routes.
5. Evaluate success and feeling, not only shortest time: premature spoiler complaints, voluntary radio use, confidence/agency, friction and how much of the space the player actually explores.
6. Choose tunables from observed distributions and failure modes, with confidence intervals where sample permits. Keep per-setting accessibility options, allow on-demand help, and never impose a dwell gate.

## Merge versus production ship

Source contracts and sensible tunables may be merged into the main source branch as **unverified engineering changes** after review. They are not release-cleared until UE5.7 compilation, engine and packaged smoke tests, sight/audio animation capture, AI movement testing and first-time playtest evidence exist. Mark each beat accordingly.
