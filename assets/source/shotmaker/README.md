# SHOTMAKER RIOT GUN — EDITABLE ORIGINAL SOURCE ASSET

**Status:** generated OBJ/MTL primitive **PROXY**, not a licensed screenshot rip, a confirmed KS-23 model, an animated weapon, or final production art. Exported in ASCII; Blender or another DCC can alter all components. One OBJ unit = one centimetre; +X is muzzle direction. A separate Unreal static mesh import creates the *actual* UAsset.

The MSX2 **manual describes riot-gun expertise**, while sprite shots show **three consecutive wide blasts**. The original manual illustration is not sufficient to identify a specific firearm. Therefore this model is an **unbranded original 1995-plausible large riot shotgun interpretation**; not a definite real-life model. Physical dimensions and joint mechanics are candidates.

## Current geometry
Independent receiver, synthetic buttstock, rubber buttpad, grip, trigger guard, sights, barrel, magazine tube, three pump grooves, pump sleeve and muzzle rim. Component groups remain editable, but there is no bolt or ammunition loading simulation yet. 240 vertices; 19 named components.

## Unreal import and animation assembly
1. Blender: import OBJ, ensure axis conversion +X forward, check scale against a 178-cm body rig, reset origin and recalc normals.
2. Correct stock and hand positions using the final Shotmaker actor's **left support-hand socket**, right grip hand, cheek/stock contact, **muzzle** and pump bones. The proxy's stock/grip is NOT approved until those tests pass.
3. Replace placeholder OBJ/MTL shading with PBR 4K source / lower-resolution runtime material instances; check texture rights and UVs. Use mesh LODs, trigger guard thickness and correct realistic sight/collar dimensions.
4. In UE 5.7 import as **SM_RiotShotgun_1995** at /Game/Weapons/Shotmaker/. The C++ boss actor will prefer that asset path over cube receiver/barrel placeholders when imported.
5. Animate a 3-shot volley with pump/recoil recovery as **authored gameplay tells**, not necessarily a true replica of a particular real-world firearm. Tune after novice playtesting.
6. Record an actual in-editor screenshot of hands/muzzle/weapon, plus third-person readability and 720p lighting before marking it a passed asset.

## Alternative sourced legal candidates (not imported)
- [Quaternius Animated Guns Pack](https://quaternius.com/packs/animatedguns.html) — CC0 page, shotgun and animations; verify exact archive/release and whether its silhouette suits a 1995 Soviet specialist
- [Quaternius shotgun / Poly Pizza](https://poly.pizza/m/nusk0Rwvrc) — CC0 listing, source accuracy not established
- [KS-23 by Milch Frische](https://sketchfab.com/3d-models/ks-23-e17651446a4845b0bfbdcd10e32be672) — CC BY page, model not imported; specific identification is **unverified**
- [KS-23 by Sassiska](https://sketchfab.com/3d-models/ks-23-shotgun-4eb00431f389451fb010c9f34f5a497b) — CC BY page, model not imported

**Art approval gate:** legal provenance, unmistakable 1995 identity, retargeted animation, physical contacts/attachments and screenshot evidence. This source mesh gives us something editable, not an MGS4-quality finished firearm.


Unreal source importer: [instructions](IMPORT_TO_UNREAL.md), [single-object OBJ](riot_gun_game_mesh.obj), [Editor Python script](../../../unreal/MGRemaster/Content/Python/import_shotmaker_source_assets.py). These are source assets and script, not evidence the Editor imported or tested them.
