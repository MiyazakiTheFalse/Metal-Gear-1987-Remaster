# DOCTRINE 001 — 2D-to-3D Map Reconstruction (binding)

**Status: ACTIVE / NON-OPTIONAL, governed by [Doctrine 000 — How We Build](DOCTRINE_000_HOW_WE_BUILD.md).** All level layouts also follow [Doctrine 005 — Spatial Structure](DOCTRINE_005_SPATIAL_STRUCTURE.md). Applies to all converted Metal Gear rooms, maps, encounters, camera zones and progression changes. The goal is a faithful but fully playable free-camera third-person game. This policy takes precedence over speed, visual ambition, or asset convenience. Changes to doctrine require an explicit design-decision record.

## Immutable design invariants

1. **Original room graph survives.** Record source room IDs, entrances, exits, elevators, keys, locks, objectives and story gates. All original connections must exist in the remaster at the abstract room-graph level. Optional 3D subrooms may be inserted, but no newly added path may bypass the source progression gating.
2. **Preserve stealth functions, not pixel dimensions.** Record each room's infiltration function: enemy patrols, information available to the player, cover, concealment, puzzle, escape and item dependencies. Width, scale, height and proportions may change only to serve the modern camera, movement, cover, or believable architecture. Never delete an encounter, puzzle or traversal dependency without an approved equivalent.
3. **Camera is a design constraint from the start.** Design with the actual free-look/over-shoulder shoulder-swap camera and a crouching player. Check doorways, narrow corridors, corner turns, ceilings, traversal, aim lines, near-wall occlusion, target readability and camera-collision behaviour. Camera quality cannot be 'fixed later' with graphics.
4. **No unplayable stealth spaces.** Preserve meaningful guard sightlines, player-readable concealment, audible risk, alternate tactics where appropriate, and fair recovery after detection. Added corridors or vantage points must not trivialise the original critical stealth challenge or create unavoidable detection.
5. **Verticality serves the existing mission.** Gantries, ladders, vents and elevations must plausibly belong to the facility and must not bypass a locked critical gate. Mark original vs added routes and check both.
6. **Environmental logic and wayfinding.** Convert abstract tiles into credible military construction; doors, machinery, lighting, signage and distinctive landmarks must help orientation without confusing the source's room identities.
7. **Greybox before art.** Prove playable movement, camera visibility, stealth and progression with placeholder geometry *before* high-resolution content, elaborate cinematics or art polish.
8. **Multi-input, scalable presentation.** Keyboard/mouse and gamepad both require tested camera, movement and traversal. Performance is the default; higher visual quality is opt-in and must not change gameplay geometry, enemy logic or saves.
9. **Preserve source-of-truth and provenance.** Every authored 3D area must link to an identifiable J2ME/MSX2 source room, a comparison screenshot/video or a clearly marked *new* addition. Inferred room layouts are labelled as provisional until validated against gameplay.
10. **No unreviewed exceptions.** Changes to route structure, mission gates, stealth solution or recognition landmarks require a dated decision record explaining the original rule, why the change is needed, gameplay consequences and tests. A documented exception does not silently override automatic progression checks.

## Mandatory level handoff

Every converted area MUST have:
- A versioned level manifest in `maps/levels/*.json` with original and converted room IDs, source and remaster connections and gating requirements.
- Walkable-width, headroom and camera clearance measurements from the UE greybox (values in metres), not speculative art values.
- Named landmarks and source identity; any new room marked `added: true`.
- At least one stealth test case per original encounter; record stealth approach options and whether those tactics have been tested.
- A manual or automated review of camera near walls, corners, stairways, aim and crouch, on mouse **and** controller.
- A recorded greybox playtest and a performance check on a low-spec Windows build.
- A passed `tools/validate_level_doctrine.py` report and review sign-off before a room can be marked production-ready.

## Proposed prototype geometry targets (NOT universal industry standards)

These are *project engineering starting points* for greybox review and may be adjusted after hands-on playtests, without changing doctrine:
- Clear walk corridor width at least **1.3 metres**; where two characters must pass or fight, prefer 2+ metres.
- Indoor player headroom at least **2.4 metres** unless an intentional crouch space.
- Camera clearance allowance at least **0.35 metres**, before collision/occlusion tests.
- Target 60fps on reference PC preset; performance test required to claim it.

Width/height values alone do not guarantee good camera or traversal. The validator can check declared metadata, not real Unreal geometry: these require in-engine collision sweeps and playtesting.

## Review gates

**Draft**: source provenance documented and topology captured; no 'playable' claim.

**Greybox**: traversable 3D space; camera, physical collision, encounters and progression evaluated. Can be playtested, not called final.

**Production-ready**: source gates preserved; script/mission tests passed; camera/controller and stealth tests logged; low-spec performance acceptable; review approved.

Run locally (no paid CI required):

```sh
python -m unittest discover -s tools -p "test_*.py"
python tools/validate_level_doctrine.py maps/levels --strict
```

No `maps/levels/` production area exists at the time this doctrine is introduced; therefore an empty directory is **not** evidence of compliance. The validator rejects an empty production-level set.

## Precedents / rationale

- GDC Level Design Workshop: build and play greyboxes early; adapt to actual camera and controls: https://www.gamedeveloper.com/design/gdc-2018-level-design-workshop-an-expert-roundtable-q-a
- GDC 2017: stealth needs multipath/occlusion/verticality and combat cover/escape: https://www.gamedeveloper.com/design/gdc-2017-level-design-workshop
- Fundamentals of Real-Time Camera Design (GDC): camera requirements belong before final physical layout: https://media.gdcvault.com/gdc05/slides/GD_Haigh-Hutchinson_FundamentalsReal-TimeCameraDesign2.pdf

**Decision rule:** if prettier, bigger, more cinematic or more modern conflicts with gameplay function, source progression or reliable camera interaction, the gameplay and progression constraints win.
