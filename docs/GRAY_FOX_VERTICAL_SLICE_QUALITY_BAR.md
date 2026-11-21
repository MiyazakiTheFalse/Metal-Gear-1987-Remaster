# Gray Fox Rescue — Vertical Slice Quality Bar

**Status:** Required acceptance specification. This document is a release gate, **not** evidence that work is finished.
**Game:** Outer Heaven, 1995 — Gray Fox prison rescue. Original Connect2Media J2ME geometry/events remain pending direct verification.

## Global acceptance requirement

The entire vertical slice must pass [Doctrine 004](DOCTRINE_004_INTEGRATED_EXPERIENCE.md), with integrated beat contracts in [data/beat_contracts.json](../data/beat_contracts.json). An animation, a dialogue clip or a guard action cannot be individually signed off if its audio/visual/input/timing/mission-state relationship is untested. The beat registry currently covers 18 source-designed beats and nine provisional areas, but production assets and packaged-video evidence remain missing.

## What currently works only as source

- Unreal source: collision capsule movement, free-look over-shoulder spring-arm camera, crouch, experimental prone + wall cover, limited melee, guard/custody state, POW-led prelude, multi-hit prison walls, connected Shoot Gunner prototype, equipment/card/transmitter logic, HUD source and placeholder props. None of this has a confirmed UE5.7 build or quality playtest.
- Browser proof-of-play: original procedural human figures (simple geometric limbs), procedural materials, basic crouch, free camera and guards. No skeletal animation, prone pose, contextual wall cover, facial acting, real voice performances or finished 3D art. Browser WebGL smoke testing is restricted in the current environment.
- There are no .uasset skeletal rigs, final texture atlases, facial rigs, polished sound files or production-ready lighting/post effects committed.
- Do not call this a polished AAA-quality experience until the evidence below is recorded and a real player can complete the packaged mission.

## Animation production (priority order)

1. **Scale & retarget.** Snake uses 178cm later-game Konami reference; guards use measured human-scale variants, not arbitrary fixed 178cm. Validate skeleton joint locations, hand reach, capsule height, stair/collision alignment and ground contact against Unreal centimetres.
2. **Base locomotion.** 8-way gait: standing idle, walk, run, strafe, start/stop, pivot (45/90/180), foot plant, turn in place. Blend between gait speeds with aim/weapon/posture layers; support mouse + gamepad. Target no foot skating and no camera bob that makes aiming unreadable.
3. **Crouch.** Crouch entry/exit, crouch idle, directional shuffle, corner transitions, weapon pose, readable head/shoulder silhouette, capsule transition with no sudden teleport.
4. **Prone.** Standing/crouch-to-prone, prone idle, forward/backward crawl, lateral motion/turns, prone-to-crouch collision checks, weapon pose. Use full-body contact/collision; current 38cm capsule and swept-box proxy are **not** final prone physics.
5. **Cover.** Approach/attach/exit, shuffle along wall, shoulder-relative camera, aim peek left/right, corner exit, near-wall obstacle occlusion, optional contextual vault only if source room/pacing permits. Wall position + actor yaw must match cover surface; no snapping through scenery.
6. **Combat.** Three separate short punches with measured active frames, recovery, contacts, enemy hit-stagger and KO. Damage happens on animation notify (not on button press) once montage system exists. Do not turn 1995 Snake into Big Boss or use visually identical MGSV signature CQC by default.
7. **Character acting.** Snake listens, looks, shifts weight, breathes; Gray Fox is captive/exhausted, gestures and reacts to rescue; guard conversations and searches telegraph awareness. No lip sync until localized dialogue timing and facial rig exist.

### Integration approach

- Evaluate Epic **Game Animation Sample** for a permissibly reusable locomotion baseline under its licence, with source and engine-version provenance: https://www.fab.com/listings/880e319a-a59e-4ed2-b268-b32dac7fa016
- Retarget onto a properly licensed or independently modelled Snake/guards/Gray Fox skeleton. Do not ship third-party characters as the canonical cast if proportions/likeness are wrong. Build custom clothing, silhouettes, facial features and 1995-appropriate props from source art.
- Unreal animation blueprint should drive locomotion state via speed, acceleration, facing delta, aim, crouch, prone, cover, alerted/staggered/injury and mission interaction. Prefer rooted animations only when traversal motion warping and capsule movement are tied to ledges/doors and physical collision.
- Foot IK / slope / floor traces must be LOD-aware; feet should contact the actual walkable world (including the 3D room floor) rather than a visual-only offset.
- Do not claim animation is ready if only the prototype cylinder changes scale or tilts.

## Camera release bar

- Third-person free look, hip/aim shoulder swap, crouch/prone/cover follow, close-wall probe, sliding collision, indoor pitch limits and smooth return to default.
- Camera must not pass through walls/doors, reveal hidden adjacent rooms through geometry, bury Snake in the frame, or jump across collision on shoulder swaps.
- Test every doorway, corner, cover edge, ceiling and narrow corridor from both shoulder sides on mouse/keyboard and Xbox pad, at 30/60/120 fps. The player’s perceived aim target and projectile obstruction should agree.
- Add camera control settings: sensitivity, invert Y, shoulder preference, shake off, aim-assist strength (if any), FOV comfort and camera-distance adjustments. No automatic cinematic camera override during free interaction.

## Environment / visual art

- Confirm exact original J2ME room graph, door and object identity; retain native tile/grid dimensions separately from engineered metres. A 0.9m 3D module is not automatically an 8px original tile.
- Art direction: South African military-industrial prison circa 1995, not MGSV Seychelles Mother Base; use aged painted concrete/steel, dampness and local wear where plausible. Source evidence governs architecture and narrative props.
- Production texture budget by screen coverage, not blanket 4K: 1–2K tiling structure and hero objects as needed; trim sheets for common metal/concrete; 512–1K for small props, signage and UI where detail supports readability. These are **targets**, profile low-spec GPU memory first.
- A coherent PBR material set: albedo/roughness/normal/AO appropriately authored; distinct sound/surface responses for concrete, steel, cloth, wood and debris. No snow in this location without source/setting justification.
- Staged prison storytelling: confiscation locker, numbered cells, ventilation, electrical conduit, damaged breakable wall seams, prison beds, emergency lights, acoustic doors, guard observation spots, environmental story clues. Every object is `original_verified`, `later_canon_style`, or `remake_added`.
- Destruction: the weak wall needs cracking stage, debris impulse, dust, sound attenuation, collision transition, visible opening and persistent save state; avoid a wall simply disappearing.
- Decals: dust footprints on appropriate surfaces, scuffs, leaked water, blood only when warranted, and impact/contact marks with lifetime/LOD controls. Tracks are world events that guards can investigate if they're meaningful to stealth.
- Use source-aligned lighting to guide attention, not UI arrows for everything; ensure high contrast for interactables and people in dark zones.

## Sound / music

- Separate 3D footsteps by material, cloth movement, stance noise, impacts, wall breach, guards' boots, search shouts, environmental hum, fan, intermittent machinery, alarm, UI select/denied, ear-level stealth cues.
- The original J2ME MIDI provides licensed reference material, but instrument/sample rights and remaster distribution clearance still need verifying. Preserve motif and phrasing; re-orchestrate by suspense state rather than playing one static loop.
- The alert state comes from actual guard knowledge. Music layers/stingers: patrol, suspicion, alert, search, return. Smooth crossfades. Dialogue side-chain ducking.
- Voice first-pass: performance-marked subtitles with timing, character intention and contextual pauses. Synthetic voice samples must have a lawful voice/model licence and original identity, not an identifiable actor impersonation. Reject robotic line readings.
- Spatial audio should convey distance, occlusion and location; UI/Codec audio is intentionally dry/band-limited and separate from 3D world sound.

## AI and systemic interaction

- Navmesh or measured collision-aware patrol paths, cover-search selection, alert broadcasts with latency and actual witness reports, investigation of audible steps, broken walls, open doors and material tracks.
- Perception must use sight cones, physical line traces, obstruction, stance, lighting and sound propagation. A guard must never magically know Snake’s current position after sight is lost.
- Guards must remain within original progression constraints and cannot phase through a closed prison wall.
- Events have source ID, cause, durable state transition, visible/audio feedback, interrupt policy, save/load test and appropriate AI/world response.

## Pacing and narrative

- Prologue/tutorial can be optional, but no mandatory training checklist and no retroactive story retcon. Capture → discovery → wall breach → Gray Fox → exit breach → stealth escape → recovery.
- Tutorials occur during real play using environmental affordances, optional Codec hints and readable HUD; player agency intact. No forced 11-second dialogue stall.
- Tension curve is a *target*, not forced detection. Quiet Gray Fox reveal, then genuine guard threat along a readable route.
- Pause/selection must distinguish in-world slowed-time from a genuinely paused Codec; never silently drop player input or lose critical intel.

## Signed-off release checks

For each feature retain evidence: `test_id`, `original_source_ref`, `implemented_commit`, `platform`, `measured_performance`, `video_or_screenshot`, `tester`, `result`.

A vertical slice can be called complete only after:
- [ ] Fully modelled and rigged character assets with source/rights provenance
- [ ] Tested locomotion, crouch, prone, contextual cover, melee and basic weapon handling
- [ ] Camera clearance and collision sweeps on all mission zones
- [ ] Original source room/event comparison or explicit clearly labelled approved adaptations
- [ ] Diegetic tutorial, Gray Fox rescue and escape with no softlocks
- [ ] Navigation and guard communication tested against movement/cover
- [ ] All core interactions show visual AND audible feedback; no silent denial
- [ ] Distinct surface footsteps, room ambience, adaptive music and polished subtitles
- [ ] Checkpoints and control mapping test on Xbox controller and keyboard
- [ ] Performance tested in packaged **Windows** build on a modest PC
- [ ] Every shippable beat has a validated Doctrine 004 contract and recorded synchronized input/video/audio playtest evidence, including failure and replay paths
- [ ] Accessibility checks for subtitles, controls, HUD and camera options

**Non-goal:** “AAA” is not a rendering preset. Prioritise consistent physical interaction, precise animation and camera feel, usable feedback and authored pacing ahead of arbitrary polycount or texture resolution.
