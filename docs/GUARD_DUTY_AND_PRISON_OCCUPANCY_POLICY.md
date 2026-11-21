# Guard Duty and Detention Occupancy Policy — ALL MISSIONS

**Status: binding on every numbered mission, under Doctrine 005.** Source values: 1987 MSX2 game is comparative, target Connect2Media J2ME v3.3.0 has not been decoded. *An assigned guard does not mean that actor has been compiled and tested in Unreal.*

## How many prisoners are in the Mission 2 prison?

**Four visible detainees** in the current source blockout: **Snake**, **Gray Fox**, and **two anonymous background detainees** in an explicitly connected pair of barred holding bays. **Only Gray Fox is a source-anchored nonplayer prisoner here**. The anonymous two are optional remake atmosphere: not claimed to be in the 1987 scene, not interactable, not collectible, not rank-bearing, not a replacement for original POWs, and they require J2ME map review before final release. The separate 3F prisoner in Mission 1 is another area and not included in these four. **Ellen's later rescue is likewise a different story beat**, not an extra prisoner secretly occupying this scene.

The original game has prisoners distributed over several buildings. An MSX2 password/prisoner guide indexes 22 rescue entries, with variant and scoring caveats, but **that is not a reliable canonical headcount for our exact prison or target edition**. Do not crowd the secret solitary cell with anonymous NPCs simply to suggest population. The expanded side cells let Snake see the broader detention operation from the escape corridor, while Fox's hidden cell remains private and suspenseful.

### Spatial placement

| Subspace | Visible captives | Gameplay/identity | Guards |
|---|---:|---|---|
| Snake's confiscation cell | 1 — Snake (player) | Initial disarmed confinement, discover altered wall | No permanent guard arbitrarily standing in his cell |
| Hidden Fox cell | 1 — Gray Fox | Rescue and nuclear-warning interaction, persisting to later missions | No unexplained prison boss standing inside |
| North holding bay A | 1 — anonymous | Ambient detained person in barred cell, visual-only placeholder | G01 has an observation route outside |
| North holding bay B | 1 — anonymous | Separate ambient detained person, no rank/dialogue reward | Same G01 observation circuit outside |
| Property locker / escape corridor | 0 | Guarded confiscated gear and physical escape landmark | G02 conducts custody inspections and access control |

**Four detainees in four distinct roles/places; two working guards.** Two existing guards were reassigned to real posts; there is **no extra surprise squad** triggered by rescuing Gray Fox. The anonymous figures are still crude static geometry, **not** realistic rigged/animated character models.

## Guard-placement rule

> **A guard exists where there is a reason for a guard.** They must have something to watch, protect, move, check, operate, escort or investigate. That duty must be visually readable, grounded in the room's architecture and worth an enemy's paid time. Tension should come from a believable working routine and Snake's agency, not from sprinkling bodies along a line.

**Placement contract mandatory:** Each deployed guard has a stable ID, defined numbered mission, physical area and floor, protected asset, duty and inspection points, route or fixed post, facing focus, observation/handoff rhythm, plausible shift constraints, local alarm/hearing/line-of-sight, incident response, recovery and revisit state. Document provenance and how the player could recognise that duty, not merely the patrol polyline.

**At-source runtime:** `AMGGuardActor::ConfigureDuty` attaches a named duty and protected-asset ID, a specific assignment anchor, inspection dwell, and a physical focus point. `ConfigurePatrol` supplies the authored reachable work route. Idle sentries inspect their assigned object and change patrol waypoint with a brief pause. During real alarms and noises they investigate, then resume local work; they must never snap to Snake's coordinates without perception or path through a sealed wall. Patrols on another floor continue rather than freeze. A real blocked route produces a logged content defect, not a teleport or phantom chase. This system is still simple UE source, **not a completed behaviour tree/NavMesh solution**.

### Nine active authored guard assignments

| Current level | ID | Assigned real-world duty and focus |
|---|---|---|
| Mission 1: insertion | G01 | Observe inbound access and first crates |
| Mission 1: first yard | G02 | Inspect trucks and loading zone |
| Mission 1: first yard | G03 | Perform Card 1 truck loading/departure duty |
| Mission 1: second yard | G04 | Inspect guarded vehicles / supplies |
| Mission 1: lift | G05 | Access control for Building One elevator |
| Mission 1: 2F | G07 | Service and maintenance corridor inspection |
| Mission 1: 3F | G08 | Inspect surveillance gallery and respond to local CCTV |
| Mission 2: northern bars | PRISON_G01 | Observe occupied cells and periodically inspect bars |
| Mission 2: property/exit | PRISON_G02 | Protect stored confiscated equipment and exit route |

G06 was proposed for a **sealed, optional maintenance wing** but is not active. Mission 3 revisits previously authored posts; returning to a level must *not* create two copies of the same guard. The player should see stable shift patterns or a reasoned change following an escape, not unexplained new enemies.

### Mandatory for Missions 3–8

The [numbered mission file](../data/1995_numbered_missions.json) and [guard and census contract](../data/1995_guard_duty_and_prison_occupancy_contract.json) bind guard-placement obligations for the remaining original events: rooftop airspace, checkpoints, research and engineer custody, Ellen's hidden basement cell, hostage protection, bunker approaches, the TX-55 chamber and the collapse evacuation. Counts and coordinates for unbuilt areas **remain uncommitted** pending confirmed J2ME room geometry. Never invent a fixed garrison number for empty unmodelled levels.

As the player revisits a room, authorised changes must follow its real history: a previously broken cell wall, a rescued detainee, missing confiscated equipment, a CCTV outage, a heightened search or an alarm evacuation. If a new enemy appears, show an approach/shift handover or log the authored source trigger. A nonalert area may rightly be **unguarded** to create meaningful stealth relief.

### Acceptance and build gates

1. Every active `SpawnActor<AMGGuardActor>` must be followed by `ConfigureDuty` and a navigable `ConfigurePatrol` or a justified fixed assigned post.
2. The duty's focus target and inspected asset must physically exist and be reachable/visible from its work route. No patrolling through the truck meshes, cell bars, locked doors or elevator shafts.
3. A guard can continue to inspect their own floor even while Snake is elsewhere; hearing through floors is blocked and alarms decay through local search rather than being erased.
4. Optional new prisoners must not be confused with original mandatory rescues, change rank, add a false character biography or reveal lore out of order.
5. Playtest unseen route, alarm, interruption, return, camera shoulders, collision and low-frame-rate behaviour with real Unreal AI instrumentation.
6. The contract's structural Python test is `tools/test_guard_duty_contract.py`; it does **not** certify the compiled level.

**Next implementation improvements:** source-verified map topology and total POW census, skeletal mesh and idle/observational animations for each prisoner, actual staff transitions (escort, meal/property shifts), persistent knocked-out guard state and a genuine navigation graph. The visible background detainees are deliberately **removable** until the original edition is checked.

**Comparative references:** https://www.dougbabcock.com/metal-gear-prisoners.php (indexed MSX2 prisoner entries); https://gamefaqs.gamespot.com/msx/578853-metal-gear/faqs/34830 (rank/rescue and location details). These support distribution and rescue mechanics, not our exact new prison dimensions or optional civilians.
