# Metal Gear 1987 remake — complete campaign execution checklist

**Audit 20 November 2026 · source: original 1987 MSX2 game; NES deviations excluded.**

This is a **production work checklist**, **not** a claim that all original tile coordinates have been mapped. Source-only prototypes have not been UE5.7 compiled, observed in PIE or signed off as production ready. All real-size door coordinates, soldier spawn transforms, camera masks and tested hide spots are currently explicit gaps in the [source ledger](../data/FULL_CAMPAIGN_BEAT_BY_BEAT_IMPLEMENTATION_LEDGER.json). We must verify the original screen grid before allowing any entry to become canonical-ready.

| Measured scope | Current count |
|---|---:|
| Production missions | 10 |
| Individual story/gameplay beats | 114 |
| Source-prototype beat entries (some compressed/adapted) | 27 |
| Areas requiring actual production 3D topology and enemy mapping | 52 |
| Return-visit change contracts | 10 |
| Engine-tested / production-ready beats | **0 / 0** |

**Completion pipeline:** `[ ] designed -> [~] source prototype -> [ ] compiled / playable -> [ ] actually playtested -> [x] production ready`. In this document, **[~] does not mean feature complete**. Every beat is evaluated separately against the 20 dimensions in the JSON ledger: source edition, physical architecture, forward and reverse route, guard deployment/AI, vertical and crouch spaces, cover, body handling, story flags/items, persistence, realistic models, animation, materials, audio, both engines, regression and unfamiliar-player testing.

## P01: Building 1 first-floor insertion and equipment

Current status: **partial source**. Original game does not name these missions as chapters; they are our production grouping.

- [~] **P01.brief** — Optional opening radio (`B1.1F.ENTRANCE`) · source: `MGPlayerCharacter.cpp`, `game.js`
- [~] **P01.entry** — Two-guard crate-lined entry (`B1.1F.ENTRANCE`) · source: `MGGrayFoxMission.cpp`, `game.js`
- [~] **P01.trucks** — Original three supply trucks (`B1.1F.TRUCKS`) · source: `MGGrayFoxMission.cpp`, `game.js` (compressed geometry; not UE verified)
- [~] **P01.ration** — Western ration pickup (`B1.1F.TRUCKS`) · finite healing/save source: `MGGrayFoxMission.cpp`, `MGEquipmentComponent.cpp`, `game.js`
- [~] **P01.card1** — Card 1 in centre truck (`B1.1F.TRUCKS`) · physical item/source flag in both builds; later original Card-1 door still incomplete
- [~] **P01.scope** — Binoculars in eastern truck (`B1.1F.TRUCKS`) · item pickup, optical zoom, save source; not UE PIE verified
- [ ] **P01.mask** — Card 1 and sleeping guard to Gas Mask (`B1.1F.GAS`)
- [~] **P01.handgun** — Authentic handgun acquisition (`B1.1F.ARMORY`) · source: `MGGrayFoxMission.cpp`, `game.js`
- [ ] **P01.mines** — Guarded mine-truck pickup (`B1.1F.ARMORY`)
- [ ] **P01.cover** — Crate concealment and patrol avoidance (`B1.1F.TRUCKS`)
- [ ] **P01.elevator** — Two sentries and lift to floor three (`B1.1F.ELEVATOR`)
- [~] **P01.freight** — Optional remake-only working cargo conveyor (`B1.1F.TRUCKS`) · source: `MGGrayFoxMission.cpp`, `game.js`

### Facility route and environmental obligations

- [ ] `B1.1F.ENTRANCE` — linked to `B1.1F.TRUCKS`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.1F.TRUCKS` — linked to `B1.1F.ENTRANCE`, `B1.1F.GAS`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.1F.GAS` — linked to `B1.1F.TRUCKS`, `B1.1F.ARMORY`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.1F.ARMORY` — linked to `B1.1F.GAS`, `B1.1F.ELEVATOR`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.1F.ELEVATOR` — linked to `B1.1F.ARMORY`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**

## P02: Building 1 third floor, second floor and earned backtrack

Current status: **design only**. Original game does not name these missions as chapters; they are our production grouping.

- [ ] **P02.cctv** — Cover and live camera sightlines (`B1.3F.CAMERA`)
- [ ] **P02.ammo** — Original handgun ammunition (`B1.3F.CAMERA`)
- [ ] **P02.pow1** — Third-floor prisoner one (`B1.3F.CAMERA`)
- [ ] **P02.gas** — Original toxic room and mask (`B1.3F.CAMERA`)
- [ ] **P02.pow2** — Third-floor prisoner two (`B1.3F.CAMERA`)
- [ ] **P02.pillar** — Moving death-pillar and optional plastic explosive (`B1.3F.HAZARD`)
- [ ] **P02.card2** — Card 2 gated original location (`B1.3F.CARD2`)
- [ ] **P02.remote** — Earn remote-controlled missile (`B1.3F.ELECTRIC`)
- [ ] **P02.electric** — Destroy electrical floor unit with guided missile (`B1.3F.ELECTRIC`)
- [ ] **P02.pow3** — Diane-frequency prisoner (`B1.3F.ELECTRIC`)
- [ ] **P02.box** — Permanent Card 2 / floor-three cardboard box (`B1.3F.BOX`)
- [ ] **P02.merc** — Four mercenaries multi-threat or original bypass (`B1.3F.MERCENARIES`)
- [ ] **P02.silencer** — Earn silencer from optional mercenary defeat (`B1.3F.MERCENARIES`)
- [ ] **P02.grenade** — Grenade launcher behind Card 1 route (`B1.3F.GRENADE`)
- [ ] **P02.pow4** — Third-floor prisoner after gun room (`B1.3F.GRENADE`)
- [ ] **P02.laser** — Second floor invisible IR beams before IR goggles (`B1.2F.LASER`)
- [ ] **P02.pow5** — Second-floor rescue and rank (`B1.2F.LASER`)
- [ ] **P02.return1** — Backtrack to ground floor with box and Card 2 (`B1.1F.REVISIT`)
- [ ] **P02.smg** — Recover SMG behind Card 2 door (`B1.1F.REVISIT`)
- [ ] **P02.pow6** — First-floor final rescue before capture (`B1.1F.REVISIT`)

### Facility route and environmental obligations

- [ ] `B1.3F.CAMERA` — linked to `B1.3F.HAZARD`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.3F.HAZARD` — linked to `B1.3F.CAMERA`, `B1.3F.CARD2`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.3F.CARD2` — linked to `B1.3F.HAZARD`, `B1.3F.ELECTRIC`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.3F.ELECTRIC` — linked to `B1.3F.CARD2`, `B1.3F.BOX`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.3F.BOX` — linked to `B1.3F.ELECTRIC`, `B1.3F.MERCENARIES`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.3F.MERCENARIES` — linked to `B1.3F.BOX`, `B1.3F.GRENADE`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.3F.GRENADE` — linked to `B1.3F.MERCENARIES`, `B1.2F.LASER`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.2F.LASER` — linked to `B1.3F.GRENADE`, `B1.1F.REVISIT`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.1F.REVISIT` — linked to `B1.2F.LASER`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**

## P03: Building 1 basement capture and Gray Fox

Current status: **partial source**. Original game does not name these missions as chapters; they are our production grouping.

- [~] **P03.capture** — Earn the original first-floor capture and controlled detention (`B1.B1.CELL`) · source: `MGGrayFoxMission.cpp`, `game.js`
- [~] **P03.confiscate** — Take and persist honestly acquired items (`B1.B1.CELL`) · source: `MGGrayFoxMission.cpp`, `game.js`
- [~] **P03.wall1** — Locate hollow first cell wall and break it through repeat blows (`B1.B1.CELL`) · source: `MGGrayFoxMission.cpp`, `game.js`
- [~] **P03.fox** — Contact Gray Fox and recover his intelligence (`B1.B1.FOX`) · source: `MGGrayFoxMission.cpp`, `game.js`
- [~] **P03.wall2** — Locate and break second cell wall (`B1.B1.FOX`) · source: `MGGrayFoxMission.cpp`, `game.js`
- [~] **P03.prison** — Evade guarded passage without recovered equipment (`B1.B1.PRISON`) · source: `MGGuardActor.cpp`, `game.js`
- [~] **P03.codec** — Optional reliable Big Boss and Schneider clues (`B1.B1.PRISON`) · source: `MGPlayerCharacter.cpp`, `codec_portraits.js`
- [ ] **P03.bodies** — Drag/stash unconscious guards with movement and discovery consequences (`B1.B1.PRISON`)

### Facility route and environmental obligations

- [ ] `B1.B1.CELL` — linked to `B1.B1.FOX`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.B1.FOX` — linked to `B1.B1.CELL`, `B1.B1.PRISON`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.B1.PRISON` — linked to `B1.B1.FOX`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**

## P04: Shoot Gunner and Card 3

Current status: **partial source**. Original game does not name these missions as chapters; they are our production grouping.

- [~] **P04.introduce** — Shoot Gunner/Shotmaker introduction without surprise hit (`B1.B1.SHOT`) · source: `MGShootGunnerActor.cpp`, `game.js`
- [~] **P04.volley** — Distinct three-shot telegraphed shotgun volleys (`B1.B1.SHOT`) · source: `MGShootGunnerActor.cpp`, `game.js`
- [~] **P04.evade** — Original escape toward gear room while unarmed (`B1.B1.GEAR`) · source: `MGGrayFoxMission.cpp`, `game.js`
- [~] **P04.break** — Physically break recovering-room door (`B1.B1.GEAR`) · source: `MGGrayFoxMission.cpp`, `game.js`
- [~] **P04.bag** — Retrieve entire confiscated inventory (`B1.B1.GEAR`) · source: `MGGrayFoxMission.cpp`, `game.js`
- [~] **P04.tracker** — Find and remove planted transmitter (`B1.B1.GEAR`) · source: `MGGrayFoxMission.cpp`, `game.js`
- [~] **P04.boss** — Source-valid defeat and alternate outcome (`B1.B1.SHOT`) · source: `MGShootGunnerActor.cpp`, `game.js`
- [~] **P04.card3** — Take Card 3 behind owned Card 2 door (`B1.B1.CARD3`) · source: `MGGrayFoxMission.cpp`, `game.js`

### Facility route and environmental obligations

- [ ] `B1.B1.SHOT` — linked to `B1.B1.GEAR`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.B1.GEAR` — linked to `B1.B1.SHOT`, `B1.B1.CARD3`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.B1.CARD3` — linked to `B1.B1.GEAR`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**

## P05: Basement dogs, spiral, Machinegun Kid and parachute

Current status: **partial source**. Original game does not name these missions as chapters; they are our production grouping.

- [~] **P05.dog** — Card 3 dog passage, real kennel behaviour (`B1.B1.DOGS`) · source: `MGSentryDogActor.cpp`, `game.js`
- [~] **P05.weak** — Explosive-only breakable weak wall (`B1.B1.SPIRAL`) · source: `MGGrayFoxMission.cpp`, `game.js`
- [ ] **P05.spiral** — Multiple 3D-connected spiral side branches and hide spots (`B1.B1.SPIRAL`)
- [ ] **P05.suit** — Bomb Blast Suit in spiral branch (`B1.B1.SPIRAL`)
- [ ] **P05.uniform** — Enemy Uniform behind secret passage (`B1.B1.SPIRAL`)
- [ ] **P05.lift** — Connected B1-to-B2 lift and same state on return (`B1.B2.GUARDS`)
- [ ] **P05.boxB2** — Reuse permanent box under genuine basement CCTV (`B1.B2.GUARDS`)
- [ ] **P05.mines2** — Sleep-guarded mine restock (`B1.B2.GUARDS`)
- [ ] **P05.powB2** — Basement hostage and rank (`B1.B2.GUARDS`)
- [ ] **P05.controller** — Guard-operated electric floor controller (`B1.B2.ELECTRIC`)
- [ ] **P05.ir** — Earn infrared goggles for later lasers (`B1.B2.ELECTRIC`)
- [~] **P05.kid** — Machinegun Kid telegraph, suppression, reload and physical cover (`B1.B2.KID`) · source: `MGMachinegunKidActor.cpp`, `game.js`
- [ ] **P05.powKid** — Second-floor or basement rescue after boss (`B1.B2.KID`)
- [~] **P05.parachute** — Earn parachute in floor-two room (`B1.2F.PARACHUTE`) · source: `MGGrayFoxMission.cpp`, `game.js`

### Facility route and environmental obligations

- [ ] `B1.B1.DOGS` — linked to `B1.B1.SPIRAL`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.B1.SPIRAL` — linked to `B1.B1.DOGS`, `B1.B2.GUARDS`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.B2.GUARDS` — linked to `B1.B1.SPIRAL`, `B1.B2.ELECTRIC`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.B2.ELECTRIC` — linked to `B1.B2.GUARDS`, `B1.B2.KID`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.B2.KID` — linked to `B1.B2.ELECTRIC`, `B1.2F.PARACHUTE`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.2F.PARACHUTE` — linked to `B1.B2.KID`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**

## P06: Rooftop Hind D, parachute, Card 4 and Tank

Current status: **design only**. Original game does not name these missions as chapters; they are our production grouping.

- [ ] **P06.roof** — Reach actual roof elevation and guarded parapet (`B1.ROOF.CORRIDOR`)
- [ ] **P06.powRoof** — Roof prisoner and ammo route (`B1.ROOF.CORRIDOR`)
- [ ] **P06.hind** — Fight helicopter with correct weapon and real vertical cover (`B1.ROOF.HIND`)
- [ ] **P06.drop** — Parachute onto connected lower landing zone (`B1.ROOF.DROP`)
- [ ] **P06.dogs** — Exterior dogs and movement routes (`EXT.TRUCKS`)
- [ ] **P06.card4** — Earn Card 4 from correct exterior truck (`EXT.TRUCKS`)
- [ ] **P06.mineDetector** — Mine detector before landmine threat (`EXT.TRUCKS`)
- [ ] **P06.returnTruck** — Truck back to B1 without losing world flags (`EXT.TRUCKS`)
- [ ] **P06.tank** — Mine-based tank encounter, real vehicle path and cover (`EXT.TANK`)

### Facility route and environmental obligations

- [ ] `B1.ROOF.CORRIDOR` — linked to `B1.ROOF.HIND`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.ROOF.HIND` — linked to `B1.ROOF.CORRIDOR`, `B1.ROOF.DROP`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B1.ROOF.DROP` — linked to `B1.ROOF.HIND`, `EXT.TRUCKS`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `EXT.TRUCKS` — linked to `B1.ROOF.DROP`, `EXT.TANK`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `EXT.TANK` — linked to `EXT.TRUCKS`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**

## P07: Building 2 industrial passages and Fire Trooper

Current status: **design only**. Original game does not name these missions as chapters; they are our production grouping.

- [ ] **P07.entry** — Cross vehicle checkpoint into industrial building (`B2.1F.ENTRY`)
- [ ] **P07.bulldozer** — Avoid and destroy advancing bulldozer using source weapon (`B2.1F.BULLDOZER`)
- [ ] **P07.antenna** — Find antenna and restore distant radio (`B2.1F.WATER`)
- [ ] **P07.card5** — Earn Card 5 from correct source room (`B2.1F.WATER`)
- [ ] **P07.powB2** — Rescue engineer-zone hostages and update rank (`B2.1F.WATER`)
- [ ] **P07.water** — Use walkable catwalk and channel routes (`B2.1F.WATER`)
- [ ] **P07.fire** — Fire Trooper flame cone and vulnerable recovery (`B2.B1.FIRE`)
- [ ] **P07.card6** — Earn Card 6 in source-correct order (`B2.2F.CARDS`)
- [ ] **P07.returnElevator** — Bidirectional lifts with correct prior alarm state (`B2.2F.CARDS`)

### Facility route and environmental obligations

- [ ] `B2.1F.ENTRY` — linked to `B2.1F.BULLDOZER`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B2.1F.BULLDOZER` — linked to `B2.1F.ENTRY`, `B2.1F.WATER`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B2.1F.WATER` — linked to `B2.1F.BULLDOZER`, `B2.B1.FIRE`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B2.B1.FIRE` — linked to `B2.1F.WATER`, `B2.2F.CARDS`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B2.2F.CARDS` — linked to `B2.B1.FIRE`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**

## P08: Building 2 Jennifer, TX-11, Dr. Madnar and Card 7

Current status: **design only**. Original game does not name these missions as chapters; they are our production grouping.

- [ ] **P08.jennifer** — Earn and contact Jennifer at 120.48 (`B2.2F.JENNIFER`)
- [ ] **P08.rocket** — Earn rocket launcher from source-required room (`B2.2F.JENNIFER`)
- [ ] **P08.madnar** — Meet original real Dr Pettrovich and rescue demand (`B2.2F.MADNAR`)
- [ ] **P08.decoy** — Distinguish impostor from real captive (`B2.2F.MADNAR`)
- [ ] **P08.tx11** — Defeat two Arnold/Bloody Brad robots with rockets (`B2.2F.TX11`)
- [ ] **P08.card7** — Earn Card 7 as one-time boss reward (`B2.2F.TX11`)
- [ ] **P08.compass** — Recover compass from Jennifer-approved stores (`B2.2F.JENNIFER`)
- [ ] **P08.flashlight** — Collect flashlight for upcoming tunnel (`B2.2F.RETURN`)
- [ ] **P08.pow** — Rescue Card 5/6-gated hostages (`B2.2F.RETURN`)

### Facility route and environmental obligations

- [ ] `B2.2F.JENNIFER` — linked to `B2.2F.TX11`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B2.2F.TX11` — linked to `B2.2F.JENNIFER`, `B2.2F.MADNAR`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B2.2F.MADNAR` — linked to `B2.2F.TX11`, `B2.2F.RETURN`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B2.2F.RETURN` — linked to `B2.2F.MADNAR`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**

## P09: Whole-world return: Ellen, tunnels, oxygen and Coward Duck

Current status: **design only**. Original game does not name these missions as chapters; they are our production grouping.

- [ ] **P09.backB1** — Return to Building 1 through Card 6 gates (`B1.B1.ELLEN`)
- [ ] **P09.ellen** — Discover weak wall and rescue Ellen without trap exploit (`B1.B1.ELLEN`)
- [ ] **P09.tunnel** — Navigate flashlight tunnel, pits and explosive route (`UNDERGROUND.TUNNEL`)
- [ ] **P09.madnarReturn** — Return to Dr Madnar with Ellen alive and get TX-55 intelligence (`B2.B1.REVISIT`)
- [ ] **P09.desert** — Use compass and uniform in scorpion desert to B3 (`B3.1F.ENTRY`)
- [ ] **P09.b3Visit1** — Enter B3 and reach B100 initially (`B3.B100.OXYGEN`)
- [ ] **P09.oxygen** — Obtain oxygen tanks from laser-camera side chamber (`B3.B100.OXYGEN`)
- [ ] **P09.backToB2** — Physically backtrack to B2 while retaining Oxygen Tanks (`B2.1F.DRAIN`)
- [ ] **P09.drain** — Traverse deep water, electric hazard and camera awareness (`B2.1F.DRAIN`)
- [ ] **P09.duck** — Fight Coward Duck without harming three POW shields (`B2.1F.DUCK`)
- [ ] **P09.card8** — Earn Card 8 and Jennifer brother's exit ladder clue (`B2.1F.DUCK`)
- [ ] **P09.restock** — Optional rations/plastic explosives before final trip (`B2.1F.DRAIN`)
- [ ] **P09.backToB3** — Return again through desert using Card 8 (`B3.1F.ENTRY`)

### Facility route and environmental obligations

- [ ] `B1.B1.ELLEN` — linked to `UNDERGROUND.TUNNEL`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `UNDERGROUND.TUNNEL` — linked to `B1.B1.ELLEN`, `B2.B1.REVISIT`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B2.B1.REVISIT` — linked to `UNDERGROUND.TUNNEL`, `B3.1F.ENTRY`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B3.1F.ENTRY` — linked to `B2.B1.REVISIT`, `B3.B100.OXYGEN`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B3.B100.OXYGEN` — linked to `B3.1F.ENTRY`, `B2.1F.DRAIN`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B2.1F.DRAIN` — linked to `B3.B100.OXYGEN`, `B2.1F.DUCK`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B2.1F.DUCK` — linked to `B2.1F.DRAIN`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**

## P10: Building 3 B100, TX-55, Big Boss and escape

Current status: **design only**. Original game does not name these missions as chapters; they are our production grouping.

- [ ] **P10.powTruth** — Rescue original identity-revealing final POW (`B3.B100.LASERS`)
- [ ] **P10.betrayal** — Big Boss's betrayal only after player earns information (`B3.B100.LASERS`)
- [ ] **P10.laser** — Avoid lethal optical cameras distinct from security CCTV (`B3.B100.LASERS`)
- [ ] **P10.gas** — Pits gas and mines behind authentically gated walls (`B3.B100.HAZARD`)
- [ ] **P10.electric** — Cross late electrified floor with real hazards (`B3.B100.HAZARD`)
- [ ] **P10.pattern** — Transmit authentic Madnar 15+1 leg sequence (`B3.B100.TX55`)
- [ ] **P10.tx55** — Place sixteen uniquely committed plastic charges in correct order (`B3.B100.TX55`)
- [ ] **P10.countdown** — TX-55 detonates and sets persistent self-destruct timer (`B3.B100.TX55`)
- [ ] **P10.finale** — Big Boss confrontation with real cover and rocket mechanics (`B3.B100.BIGBOSS`)
- [ ] **P10.cigarettes** — Source-version-verified escape timer extension (`B3.B100.EXIT`)
- [ ] **P10.ladder** — Use correct left escape ladder based on earlier POW clue (`B3.B100.EXIT`)
- [ ] **P10.credits** — Earn closing cutscene, rank, save and credits (`B3.B100.EXIT`)

### Facility route and environmental obligations

- [ ] `B3.B100.LASERS` — linked to `B3.B100.HAZARD`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B3.B100.HAZARD` — linked to `B3.B100.LASERS`, `B3.B100.TX55`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B3.B100.TX55` — linked to `B3.B100.HAZARD`, `B3.B100.BIGBOSS`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B3.B100.BIGBOSS` — linked to `B3.B100.TX55`, `B3.B100.EXIT`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**
- [ ] `B3.B100.EXIT` — linked to `B3.B100.BIGBOSS`. **Exact 1987 tile map, physical exits, elevation, guard spawn coordinates, hiding spots, low clearance, body stash and return state not verified.**

## Revisit transactions — where the world must change

- [ ] **P02.return1**: `B1.1F.TRUCKS` → `B1.1F.REVISIT`. Permanent box and Card 2 unlocked; different hiding/room access, guards remember real alerts. Test saved guard bodies, cleared posts, Card/POW state and physical reverse traversal.
- [ ] **P03.capture**: `B1.1F.REVISIT` → `B1.B1.CELL`. Kit confiscated; original mission no longer in outdoor demo-only state. Test saved guard bodies, cleared posts, Card/POW state and physical reverse traversal.
- [ ] **P04.bag**: `B1.B1.CELL` → `B1.B1.GEAR`. Genuine gear restored; transmitter influences security until player removes it. Test saved guard bodies, cleared posts, Card/POW state and physical reverse traversal.
- [ ] **P05.boxB2**: `B1.3F.BOX` → `B1.B2.GUARDS`. Previously earned box remains usable against surveillance. Test saved guard bodies, cleared posts, Card/POW state and physical reverse traversal.
- [ ] **P06.returnTruck**: `EXT.TRUCKS` → `B1.1F.TRUCKS`. Optional one-way truck return preserves all flags, doors and incapacitated-body state. Test saved guard bodies, cleared posts, Card/POW state and physical reverse traversal.
- [ ] **P09.backB1**: `B2.2F.RETURN` → `B1.B1.ELLEN`. Card 6 permits Ellen rescue through original basement wall. Test saved guard bodies, cleared posts, Card/POW state and physical reverse traversal.
- [ ] **P09.madnarReturn**: `B1.B1.ELLEN` → `B2.B1.REVISIT`. Madnar's information changes only if Ellen has actually been rescued. Test saved guard bodies, cleared posts, Card/POW state and physical reverse traversal.
- [ ] **P09.backToB2**: `B3.B100.OXYGEN` → `B2.1F.DRAIN`. Oxygen tanks physically unlock deep-water traversal. Test saved guard bodies, cleared posts, Card/POW state and physical reverse traversal.
- [ ] **P09.backToB3**: `B2.1F.DUCK` → `B3.1F.ENTRY`. Card 8 unlocks final B100 door; prior guards/surveillance should respond to actual world state. Test saved guard bodies, cleared posts, Card/POW state and physical reverse traversal.
- [ ] **P10.credits**: `B3.B100.TX55` → `B3.B100.EXIT`. Only real escape completes story; countdown, lights and guard routes remain affected by TX55 detonation. Test saved guard bodies, cleared posts, Card/POW state and physical reverse traversal.

## Cross-cutting systems — must mature in every mission

- [ ] **SYS.CANON** — Original game tile-by-tile full floor atlas, all 8 Cards, sources + J2ME variants. Current: not started.
- [ ] **SYS.WORLD** — Physical connected Outer Heaven master architecture: 1F/2F/3F/B1/B2/Roof/Deserts/B2/Building3-B100/tunnels. Current: not started.
- [ ] **SYS.ENEMIES** — Fixed area-level 3D actor coordinates, duty posts, facing, patrol routes, cover and escalation spawn maps. Current: not started.
- [ ] **SYS.REVISITS** — State-driven return variants: corpses, disposed bodies, doors, camera power, electric units, reinforcements. Current: not started.
- [ ] **SYS.STEALTH** — Drag unconscious guards, hide them, release when interrupted, guard discovery/report/recovery. Current: not started.
- [~] **SYS.STEALTH.2** — Prone/crawl spaces, dark corners, boxes, elevated concealment and alternate routes. Current: partial source.
- [ ] **SYS.CAST** — All canonical named characters and variant NPCs: realistic costume-correct PBR skinned rigs, facial anatomy, VO and animation. Current: not started.
- [~] **SYS.CODEC** — Character-specific animated talking heads with true speaking audio and original VO, optional without pausing. Current: partial source.
- [~] **SYS.COMBAT** — Realistic muzzle flash and actual player/actor hit reactions; enemy weapon-specific physics and fairness. Current: partial source.
- [~] **SYS.SAVE** — Entire game persistence/mission re-entry with automatic data migration and no flag bypass. Current: partial source.
- [~] **SYS.SFX** — Original sound design, spatial occlusion, performance VO and adaptive 1995 score. Current: partial source.
- [~] **SYS.META** — Title, mission dossier, save slots, accessibility, records and noncanon VR separation. Current: partial source.
- [ ] **SYS.QA** — Full regression, UE5.7 actual compile and PIE, 720p/4K motion/collision and human testers. Current: not started.

## Exact enemy and architectural placement acceptance

A named enemy role without a **spawn coordinate, facing direction, sight cone, patrol route, duty post, sound reaction and visibility mask** is not mapped. Every one of the 52 zones currently fails that production-level test. For each zone, replace `null` transforms in the JSON ledger with checked Unreal world-centimetre coordinates, then prove the original screen-grid adjacency, reverse path, stairs/lift/basement/roof and cover/crawl positions in PIE. For returning routes, prove which guards are still present, which bodies were moved, which surveillance devices remain disabled and which doors/explosive breaches stay open. New reinforcements require an observable cause, not a random pressure spike.

**Source-ranked references:** [The Snake Soup MSX2 walkthrough](https://thesnakesoup.org/metal-gear-msx-walkthrough/), [1987 MSX2 recorded full run](https://www.youtube.com/watch?v=ehlAMTh42Ko), [visual source maps](https://steamcommunity.com/sharedfiles/filedetails/?id=3067870732).

**Next executable critical path:** finish P01 original Card 1 / truck / elevator topology, validate full B1-F3 route before P03 capture, and integrate body dragging/rediscovery with physical hide spots and persistence in both engines. **Parallel quality gate:** obtain, inspect, re-rig and actually render realistic lawful identity-specific characters—not mere specifications or procedural dummy geometry. The game must not claim completion until both are demonstrable.
