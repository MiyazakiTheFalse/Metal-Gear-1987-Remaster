# Metal Gear (1987 MSX2) → Outer Heaven 1995 Remake
## Whole-game conversion, boss treatments and current implementation status
**Updated:** 20 November 2026 · **Canonical anchor:** the original MSX2 game, not NES alterations · **Repository status:** GitHub C++/asset/data source only; *latest branch not UE5.7 compiled or playtested*.

### Whole-game and metagame scope at the current head

The ten major conversion phases are not ten finished missions: **four have partial authored gameplay source and six remain design-only**. Two of eleven significant combat set-pieces have source prototypes (Shoot Gunner and Machinegun Kid). The full original Building 1 is not yet rebuilt; all current maps use compressed prototype topology.

**Title/metagame 20 November pass:** a working browser-source title, selectable New Game/Continue/Briefing/VR/Options, controller and keyboard menu navigation, save-aware confirmation before a new operation, optional 1987-informed character briefing, and eight original animated codec portraits are present in source. These are illustration/prototype quality and do not imply complete PBR character models, shipped music or an Unreal native title. Source + simulated-DOM input checks passed; real browser capture/Unreal engine testing remain mandatory. See [metagame production contract](METAGAME_TITLE_AND_FRONTEND_1995.md).

### Current paired milestone

Both sources connect the infiltration and prison to the west-lift parachute pickup. Browser regression coverage includes the timed service breach, optional log, Machinegun Kid and reward. The Unreal changes require compilation and a matching PIE run. The prototypes compress the original Building 1 geography. The canonical third-floor box pickup, full roof and Hind D encounter are outstanding. Browser and Unreal Shoot Gunner combat timing, art and checkpoint implementations still differ.

Machinegun Kid currently commits to an observed lane, gives a one-second warning, fires for roughly 1.15 seconds, then opens the feed cover for 1.8 seconds. Each hit requires fresh sight. The two lift motors provide readable cover. Defeating him earns access to the parachute. These are prototype timings and staging choices. The relay log is optional and the passage stays open on return. See the [paired direction and adaptation decisions](OUTER_HEAVEN_1995_CONTINUITY_TREATMENT.md#direction-for-the-two-playable-builds).

### The decision
We are developing **the entire original game** with an MGS-style third-person presentation, not a permanently isolated prison demo. Existing prison content remains the first integration testbed; next production priorities build backwards to the actual infiltration opening and forward to the rest of the campaign. The ten source-linked workstreams and eleven set-pieces are versioned in [full-game campaign contract](../data/MSX2_FULL_CAMPAIGN_CONVERSION_PLAN_1995.json). The 23 local beat contracts (22 source prototypes, 1 design-only) and 12 area contracts remain the detailed authored-prison source and are **not** a count of fully completed 1987 levels.

### Latest status and significant corrections

| Feature / sequence | Implemented in C++ or source | Missing / must not falsely advertise |
|---|---|---|
| 1995 Snake locomotion, aim, prone, cover, wall knock, melee | Native posture/camera/input, authoritative traces, animation state bridge | Exact high-quality rig, authored IK/montages and 720p motion capture QA |
| Early infiltration and intelligence | Original 3D yard/logistics, practical posts, custody guard and POW clue; **early handgun must now be physically earned** | Complete Building 1 source-order route; Card 1, mines, explosives, remote missile and SMG placement/usable mechanics |
| First firearm ownership/checkpoint | New pistol case near opening, actual Interact/pickup, ammo ownership in V4 save, V1–3 compatibility | Real loot animation, visual handgun model import, full-featured hands and reloading |
| Non-canonical stun pistol | No longer in normal inventory; explicit default-off developer setting and exclusion from shipping builds | Retire remaining test harness when physical guard AI tests no longer need it |
| Prison / Gray Fox | Capture/transfer transaction, two breached cell walls, Fox rescue, prison guard search, optional dialogue | Physical capture/transport performance, real Fox rig and recorded VO |
| Shoot Gunner (revised Shotmaker) | 3-shot spread, pump and shell reload, limited damage, physical cover and gear recovery/card logic | Approved boss model/reload animation, original alternative victory routes, whether defeat must be mandatory in authentic progression |
| Inventory / transmitter / Card 3 | Confiscation, recovery, planted tracker, Card 2 room and Card 3 | Complete original acquisition progression and all weapon families |
| Post-Card-3 | Two source-authored sentry dogs, service lane, crawl geometry, optional kennel logs, service charge, relay room, Machinegun Kid prototype and parachute pickup | Dog rig, navigation verification, finished boss rig, production staging and roof traversal |
| Surveillance CCTV | Two placed, moving analog cameras with physical visibility, acquisition timing, alarm reports, local isolator and gunfire destruction | Camera power now restores at checkpoints. Outstanding work includes polished optical cone/spotlight/audio, monitored camera feed and balance testing |
| Audio / interface | Source transceiver, state-dependent HUD, eight original animated identity portraits, browser tactical title, briefing, VR choice, options, save-confirm menu; optional sounds/voice import contracts | Production MGS-quality title cinematics, Unreal-native menu, real character-face rig/render, recorded VO and mix |
| Later campaign | Entire original item, boss, card and POW sequence is documented | No implemented Hind D/tank/bulldozer/Fire Trooper/TX-11/Coward Duck/TX-55/Big Boss fights yet |

**Meaning of implemented:** authored C++/data and source importers, not a claim of an executable or graphically finished game. The most recent movement/foot/hand IK signals also lack the real AnimBlueprint assets to produce their intended animation.

### Surveillance: use and counterplay

**Current mechanic:** A lens physically pans along a fixed arc. Its detection trace must reach the player's body without hitting a wall. In the prototype it requires sustained optical contact (about 1.25 seconds at full silhouette) before reporting an alarm, with longer exposure when Snake is concealed by posture. A local lamp and a new HUD camera-acquisition meter warn the player first. A transmission tells nearby guards the **last observed** position; it does not grant see-through-walls targeting.

**Four legitimate counterplays:** 1) Wait until the lens scans away and cross; 2) move behind an actual obstruction or inside a low service passage; 3) use the reachable eye-level electrical isolator panel while standing, not aiming or sprinting; 4) shoot the actual camera housing with a **conventional** firearm, consuming ammunition and producing normal combat noise. The non-canonical wired-dart stun test tool cannot disable electronic surveillance. There is no camera-melee knockout because the camera is mounted high.

**Not yet complete:** A real LED/servo sweep, spot cone, damaged optic material, impact sparks, electrical isolation sound, local guard response to a destroyed camera, camera power persistence, accessible disabling prompts, performance/line-of-sight tests in Unreal. **Late TX-55 laser cameras are different hazards** and must not be mistaken for ordinary CCTV.

### Original set-piece roster and proposed MGS-compatible translation

| Order | Original MSX2 encounter | Proposed 3D design and fair counterplay | Current |
|---|---|---|---|
| 1 | Four mercenaries | Pre-Fox team tactics, asymmetric posts, credible reloads, maneuver and suppression; avoid a scripted firing squad | Design only |
| 2 | Shoot Gunner / Shotmaker | Three broad, well-telegraphed blasts; pumped hulls; cover; recover gear; viable handgun/SMG/missile approaches when source-approved | Source mechanics, not UE verified |
| 3 | Machinegun Kid | Suppressive cone, line-of-fire cover degradation, earned reposition; finite burst/reload windows, no invulnerable corner | Design only |
| 4 | Hind D | Rooftop vertical cover, rotor wash, grenade-lead aiming, physically exposed aircraft vulnerability; protect original identity | Design only |
| 5 | Tank | Armour invalidates pistol spam; mines on a visible vehicle route, turret tell, escape bays, damaging collision | Design only |
| 6 | Bulldozer | Slow but consequential mass, telegraphed push and clearance geometry, grenade trajectory and readable retreats | Design only |
| 7 | Fire Trooper | Real flamethrower arc and fuel/recovery pressure, cumulative environmental risk, non-magical fire-resistant equipment | Design only |
| 8 | TX-11 / Arnold pair | Two distinct armoured positional threats, earned rocket launcher and recoverable firing windows | Design only |
| 9 | Coward Duck / Dirty Duck | Hostage safety is THE mechanic; boomerang arcs, pit warning, restraint/precise targeting and reversible routes | Design only |
| 10 | TX-55 Metal Gear | Static heavy installation and two authentic defensive laser cameras; inspect, plan and place plastic explosives in the original puzzle logic | Design only |
| 11 | Big Boss | Unspoiled identity reveal and final confrontation. Prior training and limited resources matter; no later-game MGS boss substitute | Design only |

The early mercenary set-piece isn't always counted as a named boss, so call this the **11-event combat set-piece roster**, not eleven named bosses. Do not include the NES-exclusive Twin Shot or Super Computer replacements.

### What to carry forward unchanged and what to adapt

**Keep:** Encounter identities, narrative reveals, role of POWs, keycard/equipment dependency, Gray Fox's knowledge, true TX-55 instead of NES Super Computer, the earned 1987 conclusion, and unique required solutions (e.g. physical mines versus a tank). Preserve the option to retreat where the original supports it.

**Improve:** Scale, guard and weapon position, aim/reload feedback, physical routes around old screen edges, animation interruption, responsive camera, controls, sound layers, checkpoints, clear physical danger telegraphs, accessibility. Puzzles that were instantly readable on a top-down 2D screen must now be legible through camera angle, surface damage, environmental clues and dialogue that does not spoil their answer.

**Review before changing:** Shoot Gunner's reportedly optional defeat in some walkthrough routes, historical exploit-based skips, whether MGS-style alternatives would bypass the intended equipment lesson, exact TX-55 foot explosive sequence and full route topology. No design choice gets the label *canon* without edition-specific verification.

### Production plan: dependency order

1. **Original infiltration restore:** playable first-floor hallway network, POW loop, earned Cards 1/2, mines, explosives, remote missiles, SMG, four mercenaries. The new handgun pickup is the first source implementation step; it is not a complete opening.
2. **Quality gate across current contiguous run:** real humanoid anim rigs, foot and hand IK, cover/muzzle contact, camera alerts, dog behaviour, audio and source-correct alternatives for Shoot Gunner.
3. **Building 1 continuation:** implement authentic explosive wall, Machinegun Kid, parachute and rooftop Hind D.
4. **Building 2:** tank, bulldozer, Fire Trooper, cards/hostages/gear, TX-11 and rocket launcher route.
5. **Building 3 and finale:** Coward Duck, hostages, surveillance-laser TX-55, countdown, Big Boss and escape.
6. **Whole-game integration:** coherent environmental art kit, screen transitions, dialogue, professional audio, mission save versioning, performance budget and full accessibility pass.

Every production slice must be a start-to-end playable **chain** including entry → clue → player choice → danger → recovery → reward → transition. Do not add unrelated decorative rooms, repeated unnecessary combat or endless textual lore.

### Primary independent source anchors
- [Konami MSX2 manual: canonical seven weapons](https://metalgear.konami.net/manual/mc1/mg1_msx/pc/en/page10.html).
- [Complete MSX2 recorded playthrough with boss/item timestamps](https://www.youtube.com/watch?v=ehlAMTh42Ko).
- [MSX2 boss guide, differences with NES and original tactics](https://gamefaqs.gamespot.com/msx/578853-metal-gear/faqs/35361).
- [Source edition and naming comparison](https://tcrf.net/Metal_Gear_(MSX2)).

**No compile/test guarantee:** GitHub connector edits cannot compile UE5.7, import AnimBlueprints, bake PBR materials, verify navigation or make a download build here. Physical testing and canon-signoff are mandatory before any feature changes from source-prototype to production-ready.

### GitHub execution workstreams
The scope is divided into milestone issues with explicit in-engine acceptance evidence; the overall game remains unfinished. These are production tracking issues, not completed features.

- [N01 — original Building 1, full equipment and four mercenaries](https://github.com/MiyazakiTheFalse/Metal-Gear-1987-Remaster/issues/4).
- [N02 — custody, Gray Fox, Shoot Gunner, surveillance and rigging](https://github.com/MiyazakiTheFalse/Metal-Gear-1987-Remaster/issues/5).
- [N03 — post-Card-3, Machinegun Kid, parachute and Hind D](https://github.com/MiyazakiTheFalse/Metal-Gear-1987-Remaster/issues/6).
- [N04 — Building 2 vehicles, Fire Trooper, TX-11](https://github.com/MiyazakiTheFalse/Metal-Gear-1987-Remaster/issues/7).
- [N05 — Building 3, Coward Duck, TX-55, Big Boss and escape](https://github.com/MiyazakiTheFalse/Metal-Gear-1987-Remaster/issues/8).
- [N06 — cross-cutting 1995 art, animation/IK, audio, interface and release](https://github.com/MiyazakiTheFalse/Metal-Gear-1987-Remaster/issues/9). **This runs from the first mission, not as optional last-minute polish.**

**Current code integration note:** equipment slots are now populated before save-restoration attempts, avoiding dependence on Unreal actor BeginPlay ordering. The authentic handgun is an in-world pickup stored in V4 SaveGame; the original 1987 other six weapons remain locked and cannot be falsely awarded until they have actual gameplay handlers. A non-canonical stun test asset remains in source, but is absent from normal inventory and omitted in shipping builds.
