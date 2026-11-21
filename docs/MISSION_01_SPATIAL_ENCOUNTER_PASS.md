# Mission 01: Insertion to Outer Heaven — spatial encounter pass

**Status:** authored spatial contract and static checker only. **NOT spawned in the browser or Unreal yet.** Source fidelity is pending the J2ME room/event trace. Every coordinate below is a remake-blockout hypothesis in metres, not a recovered original measurement.

## Spatial spine and route logic

```text
                       [M01-A Insertion]
                          |          \
                   low drainage     lit checkpoint
                          |          /
                       [M01-D Service]
                              |
                       [M01-E Security]--------[M01-G Facility]
                              |                       /
                       [M01-F Maintenance]----------/
```

The drainage is an alternative route around the gatehouse, not a bypass of the equipment tutorial. Security offers a dangerous direct gate and a longer audible maintenance path. Both reconnect at the facility. Backtracking works without a loading-screen topology swap. An expert can cross without picking up a gun; first-time players get observation and recovery space.

## Placement rationale

| ID | Physical role | Guard/pickup intention |
|---|---|---|
| M01-A | Exterior insertion / safe cover | G01 teaches patrol rhythm. P01 ration is optional and safe. |
| M01-B | Drain and grate | No guard in the narrow pipe; locomotion teaches prone/crawl and audibility. |
| M01-C | Lit checkpoint and truck | G02 holds barrier, G03 roves out of phase; P02 medkit rewards reading their rhythm. |
| M01-D | Storage / service door | P03 handgun is plainly visible from cover, P04 ammo tucked behind shelves. No gun is mandatory. |
| M01-E | Internal security hall | G04 hall route, G05 elevated booth create overlapping but avoidable cones. Terminal gate is one legitimate route. |
| M01-F | Generator bypass | G06 inspects generator, P05 ration compensates for noise and detour. |
| M01-G | Main facility antechamber | P06 intel is the mission exit transaction and durable checkpoint, not yet asserted as a canonical keycard. |

## Guard sensing and tactical readability

All patrol coordinates live in `data/mission_01_spatial_blueprint.json`. Guard start points and patrol nodes are inside authored room bounds, but **collision and cones are not yet physically evaluated**. Implement these states in both runtimes using a shared declarative encounter adapter: relaxed patrol → suspicious orientation → move to last audible location → localized investigation → search → return. Confirmed line-of-sight can escalate to squad radio only with plausible comms; a noise stimulus gives its source, not Snake's coordinates. Stun/drag and unconscious-body discovery should be implemented and tested against those states before production signoff.

No room-entry spawn waves. Existing enemies persist, and doors/glass/shelves must occlude sensor rays while sound attenuation depends on material and openings. Keep guard awareness across room boundaries when in audible/visible range. Make a direct route possible with zero pickups and zero alerts; nonlethal takedowns are optional, not required.

## First-time walkthrough

1. Establish distant Outer Heaven silhouette, give immediate player control and optional short radio context, show G01 crossing the approach before the player is in danger.
2. Player sees two credible routes: an exposed checkpoint and a darker drainage mouth. Neither uses a giant glowing arrow. Separate ingress landmarks by silhouette and environmental noise.
3. At checkpoint, G02 faces the barrier while G03 returns to desk. A shadowed truck edge is the waiting place; line-of-sight and footstep audio tell the same story.
4. Through either route, enter service store. First firearm can be found without being forcibly equipped. A container label and table silhouette support colourblind readability.
5. Reveal security booth and hall guard from a safe lip. Player may use terminal, distract one guard, or seek the generator detour.
6. Facility entry persists mission state. Codec provides next objective while the player retains control; no arbitrary cutscene lock.

## Expert and failure routes

- Expert: insertion → drainage → service → security → maintenance → facility; no pickup or forced combat.
- Direct: insertion → checkpoint → service → security terminal → facility.
- Detected: retreat through service into checkpoint or drainage; radio updates only guards who can receive it; no teleported reinforcement behind Snake.
- Return: facility → service path, previously opened passages stay open, collected items stay collected, guard investigation state survives allowed loading transitions.
- Low resources: do not gate a story objective behind a finite round count or a weapon acquisition.

## Shared source contract / implementation gates

1. **Verify** original source-map connectivity, source item coordinates and mission beat order from J2ME a1; annotate MSX2 differences. Rework contract before claiming canon.
2. **Geometry adapters**: browser and Unreal consume the same IDs, portals and spawn transforms; preserve a 2D local XY source, explicitly map to engine 3D axes, and keep stable IDs in save data.
3. **Placement adapters**: G01–G06 use actual NavMesh/waypoints and line-of-sight traces; P01–P06 use pickup interactables, inventory effects and persistence.
4. **Encounter tests**: at least two stealth solutions and one alarm recovery per guarded room; tests for bodies, door occlusion, no duplicated pickup, zero-weapon clear.
5. **Camera/crawl**: verify left/right shoulder in every portal at 30/60/120 FPS; ladder and crawl transitions cannot bypass locked gates.
6. **Evidence**: record scene traversal, AI path traces and video from both builds before marking any area `playable`. Static-data passing is not Unreal compilation or browser parity.

Run `python tools/validate_mission_01_layout.py` for data integrity. This does not yet establish runtime integration. Document any J2ME disagreement as a revision rather than quietly treating a speculative remake passage as an authentic 1987 route.
