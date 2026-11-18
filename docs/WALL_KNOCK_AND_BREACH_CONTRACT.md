# Wall knock / breach interaction contract

Status: Unreal Engine 5.7 source-only prototype. Not compiled or playtested.

## Infiltration controls
- Move into a solid full-height wall to enter automatic cover; sideways inputs shuffle along it. Moving away, sprinting, or going prone releases cover.
- Press F / gamepad face-bottom while in cover to knock at the wall impact point instead of punching. Otherwise normal melee remains available.
- Aim while sliding toward a wall end for a small camera corner offset. The existing camera probe still enforces collision.
- E / gamepad face-left always prioritizes mission interaction. Gray Fox prison WeakWall_A and WeakWall_B receive MG_Breachable; cover detection ignores these while intact.
- Breaching either mission wall immediately clears any prior cover state. The rescue flags, immediate interaction availability and optional 28/75-second candidate hints remain non-blocking and are now exploration-aware.

## Guard response
- Knock event contains a wall impact, outward surface normal and distance-limited sound radius, never Snake's coordinates.
- A hearing guard follows React -> Approach -> Search -> Return. Visual contact and real combat alerts override the distraction.
- Repeated knocks renew search, not magical detection. Metal, wood and default concrete use provisional different ranges; wall-side/obstacles attenuate them.
- Search uses swept movement but still lacks NavMesh route finding; obstructed investigation may turn into local search.
- Optional sounds are loaded from /Game/Feedback/knock_metal, knock_wood, knock_concrete. Missing audio assets do not disable AI response.

## Acceptance tests on an Unreal 5.7 build host
1. Start in capture cell. Approach intact WeakWall_A: no cover attachment; E immediately breaches it.
2. Rescue Gray Fox. Approach intact WeakWall_B: no cover attachment; E immediately breaches it. No forced delay.
3. Use a regular full-height wall: enter cover, slide both ways, press F to knock, move away to detach.
4. Guard turns toward impact location, approaches, looks around and returns if Snake is not seen. It never phases through solid walls.
5. Confirm melee outside cover, camera collision/peek, repeated knock cooldown, crouch/sprint/prone transitions and gamepad controls.
6. Confirm save/reload and mission progression are not blocked by cover, knocking or guard alerts.

Source authoring is not a finished rig or playable package; animation blueprint inputs are hooks for future assets.