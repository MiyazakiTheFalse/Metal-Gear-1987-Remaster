# DOCTRINE 005 — Spatial Structure and Player-Led Flow

**Status: BINDING, GAME-WIDE, subordinate to [Doctrine 000](DOCTRINE_000_HOW_WE_BUILD.md).** Complements event, pacing, integrated experience and source-geometry doctrines. This is a release requirement, not an assertion that the current greybox meets it.

## Rule

**Build the desired emotional rhythm into navigable space before relying on scripts.** Topology, transitions, observation points, affordances, sound, patrol routes, landmarks, sightlines and moments of relief are one design system. The player should learn to read the world, make choices and feel consequences rather than simply follow prompts.

## Required area/level contract

Every playable area gets an entry in `data/area_contracts.json` with a unique ID and:

1. **Source and mission invariants:** reference and verification status for original room graph and critical story gates, marked as canonical, comparative or remake addition.
2. **Arrival and orientation:** entry camera/spawn, immediate safe observation opportunities, first navigational goal, distinctive non-colour-only landmarks.
3. **Topology and traversability:** adjacency/portals, blocked paths, clearances, route variants, sightline and camera collisions, no physics shortcuts across canonical mission barriers.
4. **Visual/aural geography:** what is visible/heard and from where, material cues, occlusion and sound paths, reveal silhouettes, readability without critical colour/audio alone.
5. **Risk and rhythm:** exposure windows, guard patrol collision paths, recovery/safe zones, optional exploratory space, genuine tactical choices and intended tension change.
6. **Timing and agency:** events unlocked by world state, rather than arbitrary room-entry clock; optional hints must respect exploration. Fast/slow runs must both work.
7. **Failures, accessibility, and test route:** reverse traversal, interrupted contact, guards searching, save/load, two camera shoulders, controller, low FOV/low resolution, low frame rate; clear worst-case escape from every reachable point.
8. **Real build evidence:** measured traversal/comprehension/recovery and camera tests from unfamiliar players, distinct from mere geometry layout inspection.

## Spatial pacing principles

- **Compression and relief:** make tight cells/thresholds contrast with wider observation points and passages. A rest beat is felt because enemies and risk are causally absent, never artificially disabled.
- **Prospect and cover:** reveal enough to plan without showing every answer. A guard's patrol must be read from at least one safe vantage, with legible hesitation, orientation and direction changes.
- **Spatial foreshadowing:** materials, seams, acoustics and sightlines should teach common interactions before they become mandatory. Repeat an affordance consistently instead of inventing a one-off puzzle language.
- **Meaningful choice, not false branches:** optional detours earn information, concealment or resources; preserve source topology and event order. No new passage skips Gray Fox rescue or mission checks.
- **Physical AI knowledge:** guards react to what their senses can reach. Room walls, corners and occluders are both audiovisual and navigation design constraints; navigation must route rather than phase through architecture.
- **No theatrical input penalties:** the trigger and level geometry must allow immediate use by a knowledgeable player, and should not wait for an audio line, dramatic pause or cinematic edit.
- **Readable and accessible:** labels, height contrasts, material outlines, captions and interactable proximity cues are designed before final art, not retrofitted only after tests.
- **Test routes, not screenshots:** sample first-run cautious exploration, expert direct route, alarm-triggered improvisation, save/load, return/backtracking and alternate camera placements.

## Gray Fox prison: three-space rhythm (provisional, NOT source-confirmed map)

| Space | Player experience | Structural implementation and gating | Required proof |
|---|---|---|---|
| Snake's cell | Confinement, curiosity, safe examination | Breachable altered panel visually integrated into one wall; contrasting ordinary solid surfaces; listening positions; no auto-cover on mission breach panels. Space allows a player to inspect without an interaction arrow. | Immediate exit if solution is known, non-audio recognition route, no fake early enemy pressure. |
| Gray Fox's cell | Unexpected connection, clarity, quiet revelation | Transition reveals Fox's silhouette gradually from the opening. Enough standing and camera room for a short conversation. No new wave purely to enforce tension. | Readable identity and rescue, unblocked return path, no input gate during dialogue. |
| Escape corridor | Alert possibility, tactics, recovery | Player can observe patrol before exposure; cover is physically usable; guard approach/return routes use real collision; farther exit provides identifiable landmark. Dynamic sound carries threat based on actual location. | A stealth success route and a detected recovery route, physically feasible at 30/60/120 FPS with both shoulders. |

The current straight-line prison actors are a **functional test**, not an approved final level. Exact J2ME map geometry and story-event placement must be compared before building production art; section topology above represents intent only.

## Enforced workflow

- Before building a new area, write its area contract and link relevant Doctrine 004 beat IDs; add at least one first-time and one expert route walkthrough.
- Before replacing a greybox, verify original room evidence, collision and camera clearances; then bake audio and animation stage boundaries into the same layout revision.
- For each layout change, evaluate timing, guard sensing, save/checkpoint safety, room pressure and environmental clues together.
- Static validation ensures record completeness; level traversal, sound occlusion and player comprehension still require compiled Unreal playtests. Never call a source-only layout fully verified.

## Reference starting points

- The Level Design Book: https://book.leveldesignbook.com/process/preproduction/pacing
- GDC Level Design Workshop: https://www.gamedeveloper.com/design/gdc-2017-level-design-workshop
- Environmental and stealth level design references should be logged with dates, original editions, licensing constraints and observed implementation details; do not treat cited advice as calibrated timing data.
