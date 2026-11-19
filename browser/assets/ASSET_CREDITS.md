# Local character assets

The default Snake, human Gray Fox and patrol guard models, vertex colours, skin weights and poses were authored for this playtest. Mesh profiles and motions are editable in field_rigs.js. The GLB exporter uses the same definitions. The comparison option below uses a separate donor model.

# Rigged-character visual upgrade — source and licences

**Status:** Optional streaming rendering layer. This file provides provenance, not a claim of final 1995 Solid Snake or Gray Fox likeness. The default browser engine and its independent procedural human models still work without internet.

- **Rigged soldier body and original rig:** BAMEN, *FREE [Military Soldier] RIGGED*, [Sketchfab](https://sketchfab.com/3d-models/free-military-soldier-rigged-e9c56308a67d4a3db62e914fafa4d198) — CC BY 4.0, attribution required.
- **Retargeted soldier asset / browser production source:** [cktang88/3d-fps](https://github.com/cktang88/3d-fps), `public/assets/models/characters/soldier.glb`, pinned commit [`130ac2a`](https://github.com/cktang88/3d-fps/commit/130ac2a5c856401ec36e7d35afcfefe40c5801f4). The upstream credits describe the source body and animation provenance. Original asset not copied into this package.
- **Animation base:** Quaternius *Universal Animation Library* (Standard) and version 2 — CC0 at the time of sourced release; refer to [Quaternius](https://quaternius.com/packs/universalanimationlibrary.html).
- **Retargeted motion capture clips:** 100STYLE dataset, Ian Mason, Sebastian Starke and Taku Komura, [Zenodo](https://zenodo.org/records/8127870) — CC BY 4.0.
- **Runtime:** Three.js (MIT) including GLTFLoader and SkeletonUtils. Loaded from jsDelivr and animated in a separate transparent overlay with matched third-person camera and depth-only world mask.

The model is a **third-party generic military soldier**, not a verified Solid Snake, Gray Fox, or 1995 Outer Heaven guard likeness. The current test uses it for all three characters with different material direction; professional character-specific rigs, performance capture, bespoke clothing, facial rigging and animated interactions remain separate release gates.

## Why does it load from the internet?

The restricted build environment could verify the upstream GitHub source and model path but could not retrieve its binary for offline inclusion. The model and Three.js are loaded at runtime when internet is available. This is *not* an offline high-fidelity pack. No dependency can block the playable procedural version.

## To validate the high-detail mode

1. Use desktop Chrome/Edge and an active internet connection. Open `index.html` from the extracted folder, enable **Use rigged, animated soldier models**.
2. After load, the intro badge should report the number of model bones and animation clips. If the badge reports **OFFLINE / RIG UNAVAILABLE**, the procedural game remains active.
3. Test idle, walk, sprint, crouch, prone and fallen guards, including transitions, 30fps and gamepad. A model lacking a suitable prone clip must fall back to the procedural prone model, not an upright soldier.
4. Inspect all room corners, camera/occlusion, ground contact and controller yaw. Browser visual quality is not certified until a real GPU playtest passes.
5. For production release, download, verify and optimise model/texture size, add authored Snake/Gray Fox identities, ship the approved assets locally and remove the external runtime dependencies.

## Future model art direction

Target a younger 1995 Solid Snake with simple olive tactical fatigues, appropriate boots, silhouette, hair and period-correct equipment. Gray Fox in prison clothing should **not** look like a second fully kitted soldier. Guards need consistent 1995 military uniforms, readable weapon pose and convincing alert animations. Prioritise single coherent art direction and proper animation blending above arbitrary 4K textures.
