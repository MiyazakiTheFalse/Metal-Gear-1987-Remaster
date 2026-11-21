# Building One vertical progression and elevator implementation

**Status: Unreal C++ source pass only, not UE5-compiled or playtested.** Built on top of the provisional Mission 1 greybox. The supplied J2ME 3.3.0 edition remains the required source of truth. The MSX2 route is comparative, not definitive for the target edition.

## Three connected decks

- **1F:** Truck compounds, opening equipment and the main elevator lobby. Previously authored source-proxy layout remains intact.
- **3F:** Provisional camera gallery (visual camera proxies, not functioning detectors), handgun ammunition pickup P07 (12 rounds, tuning candidate), patrol G08, and a prisoner-contact interaction placeholder. The ammunition is not collectable from the same XY coordinates below; pickups use 3D range.
- **2F:** Service/security corridor, one assigned guard patrol G07, cover and access back to the lift. This is provisional geometry without final source-confirmed pickups or story gates.

Floor spacing is 600 Unreal units/6 metres between decks; this is a remake design value, **not a measurement of original 1987 architecture**. The elevator has the same XY shaft coordinate on all decks.

## Player experience

Initial elevator target is **3F**, matching the comparative 1987 progression toward the first handgun ammunition. When Snake is within three metres of a landing, press **Q / shoulder swap** to cycle to another floor, then **E / interact** to travel. When away from the elevator, Q continues to swap camera shoulders normally. The HUD indicates current deck and selected destination. The default next destinations are 1F→3F, 3F→2F, and 2F→1F, but the player can cycle to the other accessible deck.

On interaction, movement is temporarily disabled, any active wall-cover attachment is released, and a dark elevator-transit overlay masks the transition. After an authored 0.85-second simulated cabin phase, Snake is placed at the destination platform and movement is restored. The selected deck, pickups, prisoner-contact flag and open Card 1 door persist. Restarts after reaching the upper floors resume at the relevant elevator landing rather than an unvalidated hallway position.

**Important limitation:** the blackout is currently only a transition mask, **not a genuine asynchronous loading screen**. All 2F and 3F geometry is built during mission startup. True asset/sublevel streaming should replace the timer when cooked floor assets exist; the lift must wait for actual streaming readiness and offer a failure/retry path rather than using an arbitrary load delay. No polished elevator door/ride animation, lift audio, animation montage, streaming asset, or packaged build exists yet.

## Guard and item continuity

G07 is a service-corridor inspector, G08 is a camera-gallery patrol. These are believable fictional duties, not recovered guard coordinates. Guard logic does not sense or pursue Snake if separated vertically by more than 260 cm; this is a source-prototype rule until floors have real navigation portals. Camera visuals have **no detection gameplay** yet, and the prisoner-contact marker is not a complete rescue narrative. The existing gray-Fox rescue slice is not connected to these floors.

P07 grants ammunition to the HANDGUN slot **without granting a gun**. The save system reconstructs ammunition from the committed pickup ID; it does not spawn duplicates. Full weapon firing behaviour, subsequent consumption and complete ammo persistence still require integration and playtesting.

## Build and QA

Source checks are in `tools/test_building_one_elevator_contract.py` and `tools/test_mission_01_runtime_contract.py`; static contract in `data/building_one_vertical_contract.json`. Run on a development checkout:

```sh
python tools/validate_mission_01_layout.py
python -m unittest discover -s tools -p 'test_*.py'
```

Then **compile UE5.7 on an actual Unreal host**, open from a fresh save, use 1F→3F→2F→1F and 1F→2F→3F routes, check no collider traps, verify pickup timing on each deck, save/load while in an upper floor, test interruption during transit, test both shoulder cameras, test movement and AI before and after arrival, and verify full gamepad input. These checks have not been executed in the authored environment.

The next fidelity pass must decode source-specific third-floor rooms/cameras, the second-floor map and all other Building One elevators, then build additional floor content and real async streaming assets. **Do not call the upper floors content-complete or the elevator production-ready based solely on these files.**

References: https://thesnakesoup.org/metal-gear-msx-walkthrough/ (Building One 3F and 2F) and https://www.vgmaps.com/Atlas/MSX/ (comparison maps).
