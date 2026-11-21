# Outer Heaven 1995 — whole-site master architecture and beat atlas

**Status (21 November 2026): design-grade spatial masterplan, NOT a verified 1987 screen-by-screen reproduction or built UE level.** Master data: [`browser/atlas/outer_heaven_1995.json`](../browser/atlas/outer_heaven_1995.json). Interactive, annotated drawing: open **`http://127.0.0.1:19870/atlas.html`** using `browser/tools/serve_playtest.py`, or run the browser launcher and replace the URL with `/atlas.html`.

**Coverage contract:** 52 existing ledger areas · 114 **actual named ledger beats** · 10 mission workstreams · 11 boss/set-piece nodes · area-to-area proposed connectors · annotated pickups and doors. No new invented narrative progression is substituted for the 1987 MSX2 material. Coordinates are **a spatial layout proposal**, not recovered MSX2 room or guard coordinates; the current browser/Unreal prototypes still use their earlier compressed layouts, so neither build yet physically matches this campus drawing. This document describes the target for level-authoring.

## Canon and inference boundary

The original 1987 MSX2 manual illustrates **three principal buildings** and internal floor movement via lifts, with original play driven by security cards, POW intelligence, patrols, equipment and escalating bosses. The 1987 schematic represents Building 1 (1F/2F/3F/BF/R), Building 2 (1F/2F/BF/R) and Building 3 (1F), but **not surveyed metre dimensions** or a comprehensive modern free-camera level. See the contemporary MSX2 manual illustrations and Konami's official material:

- MSX2 1987 manual, including *Outer Heaven* schematic and equipment pages: https://www.ne.jp/asahi/hzk/kommander/mg1manual.pdf
- Konami archive (original game): https://www.konami.com/mg/archive/mg/intro.html
- Source game-specific route and set-piece review: `data/MSX2_FULL_CAMPAIGN_CONVERSION_PLAN_1995.json`; beat-by-beat status: `data/FULL_CAMPAIGN_BEAT_BY_BEAT_IMPLEMENTATION_LEDGER.json`
- Public real-world architecture precedents, **not canonical reference sites**: `data/real_world_architecture_reference_ledger.json`

**Strict exclusions:** Do not use the NES "Twin Shot"/"Super Computer" material as MSX2 authority. Do not mix MGSV facilities, technology or later-game identities into an allegedly canonical 1995 building. The source does **not** provide a geodetic survey of South Africa; the highland/scrub exterior and every authored dimension are new art and engineering assumptions. Do not present the atlas as a secret recovered map.

## Whole campus: top-down masterplan

This is the high-level massing and travel logic, with **metres** in the interactive plan. North is **up**, not a statement of canon cardinal directions.

```text
                         NORTH / scrub / rocky outcrops
      Watchtower        helicopter infrastructure                 Watchtower
  +-----------------------------------------------------------------------+
  |           LANDMARK: BUILDING 1                Power / water      B3   |
  |         +-----------------------+       +------------------+  +-----+ |
  |         | SECURITY, COMMS, POWs |       | B2 INDUSTRIAL/R&D|  |     | |
  |         | 1F · 2F · 3F · roof   |       | loading, fuel,   |  |core | |
  |         | BF · deep services    |       | mechanical bays  |  | and | |
  |         +-----------------------+       +------------------+  |cmd  | |
  |                                                                 +-----+ |
  |       VEHICLE YARD / FREIGHT       EXPOSED TANK TRACK / BLAST WALL     |
  |           staging      8m service road / guarded crossings            |
  +------ dual-gate -------------------------------------------------------+
      Watchtower           SUPPLY APPROACH / OUTER PERIMETER       Tower
```

**Exterior logic:** The facility should appear to have been expanded over decades: older reinforced-concrete shells; added 1980s metal gantries, low-security service sheds, water and diesel infrastructure, cables and utilities, cargo ramps, guard walks, floodlights, blast doors, drain ditches, weathered warning stripes and repair patches. Each tower has a specific overwatch field and corresponding credible blind spots. Architectural form is driven by a real logistical purpose rather than generic infinite concrete halls. At the distant horizon show a severe, isolated military-industrial landscape, not a town with anonymous skyscrapers.

**Site massing targets (unverified):**

| Element | Proposed metric | Why / review gate |
|---|---:|---|
| Local site survey rectangle | **990 × 580 m** | Supports plausible outdoor distance; does not force the player to traverse all of it |
| B1 footprint | **235 × 221 m** | Administrative/security and prison wing with separated lift shafts; high massing, sparse playable routes |
| B2 footprint | **214 × 210 m** | Vehicle, manufacturing, fuel and research hall coexist in a believable structural grid |
| B3 footprint | **196 × 210 m** | Surface command building and deep restricted laboratory; not all basement space is playable |
| Security wall | **6.4 m high** | Blocks casual exterior visibility; camera collision and exterior view lines must be checked |
| Watchtowers | **12.5 m high** | Line-of-sight / alarm threat, with believable lower-level blind angles |
| Freight / fire access road | **8 m clear** | Two-way operational vehicles; B2 tank clearance and escape visibility |
| Freight turning area | **20 m** | Truck docking and 180° turning without props clipping |
| Heli support slab | **47 × 47 m** | Proposed campus logistics; distinct from gameplay Hind D boss roof |
| Standard floor-to-floor | **4.5 m** | 3 m clear headroom, services and structure; adjusted per industrial bay |
| Normal patrol corridor | **2.2 m clear** | Human passing, shoulder-camera collision, body drag, stealth counterplay |
| Intentional chokepoint | **1.5 m minimum clear** | Never narrower by default; retest crouch and camera |
| Prone-only utility duct | **0.85 m nominal** | **Must** be sweeptested with the actual player body/camera |

The project currently operates within much smaller browser and Unreal prototype coordinates. **Never apply a blanket scale factor to current actors**. A world-to-campus transform must be authored per connected level with seam/link verification; save data should continue to identify stable area/beat and world state rather than silently multiplying old positions.

### Silhouette and sightline doctrine

1. **Arriving at freight:** The east lift bank and security building read through the loading frames; B2 industrial chimneys are visible in the distance, but the building remains inaccessible by the real gated route. The guards have plausible gate duty, loading supervision and changed posts on return.
2. **B1 upper storeys:** Looking across balconies reveals the same yard and west roof platform already seen from below. Exit routes and lift wells align vertically, not by unexplained teleporting.
3. **Roof Hind D:** Provide an open aircraft firing lane with continuous roof parapets, a service rack and believable cover. Helicopter rotor exposure changes sound, dust and antenna sway. No fabricated roof-sized helicopter landing pad is mandatory in the player duel.
4. **Tank approach:** The player crosses from B1 into a real, roughly 8 m vehicle way with three mine-placement opportunities, 15–25 m sightlines at turns and hard covers that do not give free invulnerability.
5. **B2 → B3:** The player sees B3 before entering it. Restricted groundwater and cables explain the underground approach. Gates and lifts make the facility feel like a coherent secured network, not three unrelated video-game maps.
6. **Final escape:** Broken alarms, emergency light, open blast doors, smoke, falling debris and accelerating machine shutdown change *previously known* spaces. Geometry cannot trap the player if a prior boss or item is already completed.

## Floor stacks and functional zoning

These are assignments from the **actual existing 52-area work ledger**, with their provisional room placements drawn as floor overlays in the interactive atlas.

| Floor | Functional areas and level design |
|---|---|
| **B1 · 1F** | Insertion, guarded freight/trucks, original ration / Card 1 / binocular rewards, gas-control, armory, east lift. Card-gated backtracking allows the same loading bay to become a recognizable changed security space |
| **B1 · 2F** | Restricted laser systems and physically separable parachute / west lift. Select passages can be overlooked through grilles; ducts do not bypass source progression |
| **B1 · 3F** | CCTV banks, electrical hazard, Card 2 / cardboard box, Four Mercenaries, grenade launcher. Authentic equipment earning precedes the relevant encounter, not gift-wrapped at boss entrances |
| **B1 · BF** | Custody cells, Gray Fox and two real weak walls, guarded prison passage, Shoot Gunner shotgun arena, genuine confiscated gear, transmitter, Card 3, kennels, spiral stair and late Ellen rescue/revisit. Detention walls isolate sound until breached |
| **B1 · deeper service** | Guards, electric equipment and Machinegun Kid arena. **Proposed extra vertical subdivision**: validate whether it should be folded into the original BF footprint before asserting canon |
| **B1 · roof** | West-lift corridor, Hind D exposure/cover, controlled exit/drop, Card 4 mission sequence. Boss is a real aerial threat with physical projectile/LOS source |
| **B2 · 1F** | Tank aftermath entry, Bulldozer bay, water/vehicle maintenance and return drainage. Current project ledger places Duck in B2 1F; **source conversion guide describes Duck in Building 3**; this conflict is explicitly unresolved and must be reconciled from 1987 footage before spatial sign-off |
| **B2 · 2F** | Card 5/6-controlled offices, Jennifer resistance contact, TX-11 pair, Dr Madnar research/holding, return circulation; lifts and live cards keep revisit routes intelligible |
| **B2 · BF** | Fire Trooper fuel hazard, restricted service cables and later operational return |
| **B3 · 1F** | Hardened late-game entry and physically plausible access down to sensitive research rather than an instant boss room |
| **B3 · 'B100' deep core** | Oxygen/deep water, laser cameras, hazard transition, TX-55 industrial-scale chamber, Big Boss truth/fight and escape shaft. **B100 is a game/ledger label; the drawing's -20 m is a proposal, NOT a claim of exactly 100 levels or a 100 m shaft** |
| **Underground connection** | B2/B3 service tunnel with water/power trunking, emergency gate, sound occlusion and return-state changes |

## Boss arena placement: original roles preserved

| Boss / major encounter | Ledger zone | Staging, gameplay and credible scale |
|---|---|---|
| Four Mercenaries | `B1.3F.MERCENARIES` | Multi-approach squad room, layered patrol sightlines and retrievable cover, not unavoidable firing squad |
| Shoot Gunner / Shotmaker | `B1.B1.SHOT` | Confiscated-equipment recovery, short telegraphed shotgun lanes, concrete sightline breakers, physically credible weapon pump/reload |
| Machinegun Kid | `B1.B2.KID` | Enfilading defended service room, visible belt/reload window, physically navigable low duct or flank, no magic immunity |
| Hind D | `B1.ROOF.HIND` | Large roof/air volume, believable flying approach, hard parapet cover, original grenade launcher dependency |
| Tank | `EXT.TANK` | Real tracked-vehicle avenue, three mine track sites, fair telegraph and no arbitrary handgun damage to armour |
| Bulldozer | `B2.1F.BULLDOZER` | Industrial freight bay and hazard line, hydraulic startup, explosive counter, non-trapping retreat |
| Fire Trooper | `B2.B1.FIRE` | Fuel store/service room; sweeping flame and oxygen, readable cover and persistent burn zones |
| TX-11 / Bloody Brad pair | `B2.2F.TX11` | Parallel armoured flanks, rocketed weak points and earned pacing relief; not modern cyborg ninja retcon |
| Coward Duck / Dirty Duck | `B2.1F.DUCK` (**conflicted**) | Hostages / boomerang arcs / readable pit and hostage-safe fire; **floor/building must be source-reviewed** |
| TX-55 Metal Gear | `B3.B100.TX55` | Industrial rail-crane/maintenance chamber, authored planting points, real security lasers and explosion sequence; do not replace it with NES Super Computer |
| Big Boss | `B3.B100.BIGBOSS` | Escape route reveal earned through story and spatial clues; no fake final boss arena teleported out of the fortress |

## Pickups, dependencies and revisits

Original 1987 keycard placement is **not fully verified**. The atlas records **intended gate dependencies** so artists cannot accidentally open a shortcut, but speculative exact coordinates are not canon.

| Item/gate | Proposed functional location / use |
|---|---|
| Ration, Card 1, binoculars | Original Building 1 truck depot: actual physical pickups already in the current prototype |
| Handgun, mines, plastic explosive, gas mask | B1 first-floor armory/gas workstream; some original 1987 source pickups still unimplemented |
| Card 2 | Building 1 upper floor; required for original Card-2-locked **permanent** cardboard box, distinct from the remake-only temporary freight carton |
| Grenade launcher | Building 1 upper supply zone; required for Hind D rooftop |
| Supressor/remote missile | Restricted B1 security/electrical returns; do not grant if 1987 source hasn't yielded them |
| Confiscated gear and transmitter | Real basement locker and selected equipment inventory; recovery must restore exact finite weapons and earned card state |
| Card 3 | Secured B1 basement records storage; opens service route to kennels/spiral/roof |
| Parachute | Machinegun Kid/west-lift chain; intentional transition |
| Card 4 / mine detector | Roof and armoured approach progression; original pickup tile remains to be verified |
| Cards 5 and 6 | B2 industrial/security floor circulation, not free opening at mission transitions |
| Rocket launcher / Card 7 | Jennifer/Dr Madnar narrative gate before TX-11 and late returns |
| Card 8 / oxygen | Late hostage/deep drainage chain before B3 core, exact location to be validated against the source |
| Plastic explosives / TX-55 sequence | Earned ammo and physically readable planting regions, no gun-bullet damage shortcut |

**Revisit design:** Backtracking should change alarms, fallen bodies, repaired or breached walls, shift changes, doors, light power, intel earned, and shortcut accessibility. The player should recognize **which building and floor they have returned to** through unique sightlines, signage, service function and physical landmarks, not an omniscient floating waypoint. Every locked passage also needs a reciprocal safe exit after a requirement has been met.

## Encounter density, pacing and travel budgeting

These are **measurement hypotheses** for playtests, not historical game timing. Use the actual 1987 MSX2 encounter spacing and contemporary novice/experienced player observation to revise them.

- **Fresh threshold:** 8–20 s to read a recognizable unique landmark, see an authentic hazard or patrol cue, and choose stealth or speed. Avoid a surprise hit on room entry without spatial forewarning.
- **Stealth loop:** 25–60 s between meaningful decisions (cover, guard timing, distraction, card, intel), not 60 s of an empty road.
- **Pressure release:** 15–45 s after a major boss or earned revelation to examine loot, recover or reach a reversible shortcut without an arbitrary forced radio interruption.
- **Boss arena prep:** at least one readable safe observation point, an explicit commitment threshold, telegraphed attacks, physical retreat/counterplay and a non-punitive retry from a genuine saved checkpoint.
- **Normal connected traversal:** approx. 15–60 s; **over 90 s** of mandatory travel with nothing observable/useful is flagged for redesign. Shorthand distance alone is not gameplay time.
- **World revisit:** changed patrol purpose and alternate legal routes reduce repeat travel without bypassing Card 1–8 access progression.
- **Escape phase:** previously visible blast doors and power lines must transform in place rather than spawn unrelated new architecture. No late cinematic may silently reset injury, keys or enemy world memory.

## Data architecture and rendering provenance

```text
Source 1987 manual / MSX2 maps / direct play evidence
             ↓ (manual reconciliation, not fabrication)
FULL_CAMPAIGN_BEAT_BY_BEAT_IMPLEMENTATION_LEDGER.json  [114 beats]
             + data/MSX2_FULL_CAMPAIGN_CONVERSION_PLAN_1995.json [11 bosses]
             ↓ (designed metres/rooms; original room IDs unchanged)
browser/atlas/outer_heaven_1995.json [52 zones, floors, site, links, gates]
             ↓
browser/atlas.html + browser/atlas/map.js [interactive floor map]
             ↓ (human review required)
browser + Unreal actual level assets, nav/collision, guard posts, saves
             ↓
tests, camera sweeps, PIE and side-by-side novice/veteran playtest
```

A **zone** owns the ledger area ID, mission ID, known beat IDs/names, proposed centre/footprint, floor, boss, pickups, gate dependencies, proposed links and acceptance notes. A **connector** owns endpoints, vertical/horizontal type, required item/flag and canonical/proposed review status. Site features carry metre bounds, height, appearance and physical purpose. The atlas is thus a machine-readable design model, **not a replacement for a finished `.umap`**. Unreal coordinates are centimetres in authored 3D; browser coordinates are metres in the existing prototype. Use reviewed per-level transform metadata to map these onto a common world; do not derive a hardcoded 10×/100× transform.

## Acceptance and blocker register

1. **Source check:** Link each atlas connection/room to the actual 1987 tile or video, including early four-mercenary timing, Coward Duck location discrepancy and B1 deeper floor; unverified remains unverified.
2. **Geometry + sightline:** Walk from delivery truck to B1, via original card gates, down to prison, out to roof, across the tank road, B2/3 and back; annotate collision sweeps and time per seam; a floor below cannot be reached through a wall.
3. **Verticality:** Continuous lift shafts, stairs, roof parapets, ladders, deliberate prone tunnels; test 1.8m Snake physical body, weapon attachments and third-person camera with both shoulders.
4. **Enemy duties:** Associate existing `UNREAL_PROTOTYPE_ENEMY_DEPLOYMENTS_1995.json` guards with reviewed rooms **only after** a vetted coordinate transform. Verify visibility, hearing, corpse persistence and disabled CCTV on return.
5. **Items/capture:** Card-2 box, physical gear confiscation/recovery, finite inventory and boss prerequisite route cannot become softlocked by skips or shortcuts.
6. **Performance and experience:** Localhost browser 720/1080p, Unreal 5.7 actual PIE, packaged Windows build, controller, audio/visual legibility, novice confusion timing and 60fps target on specified hardware.
7. **Production signoff:** Distinct, realistic 1995-appropriate models, materials, animation, sounds, cinematics and story delivery, **not** merely the 52-area JSON and SVG.

**Do not claim a complete spatial build simply because this 52-area masterplan exists.** Current progress remains 27/114 source-prototype beats and 0 actual Unreal-verified / production-ready beats until physical level authorship and test evidence are recorded.
