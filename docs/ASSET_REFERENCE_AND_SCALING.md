# Asset Reference & Dimensional Doctrine

**Binding companion to DOCTRINE 001.** Do not scale original room geometry to accommodate an unrelated borrowed model. Fit each asset to verified room measurements and demonstrated camera/collision requirements.

## Which source controls which decision?

1. The selected original J2ME build controls rooms, doors, elevators, objectives, hazards and event progression. Original MSX2 maps can triangulate discrepancies, but the editions must not be silently conflated.
2. Konami's Metal Gear 2 profile gives Solid Snake 178 cm / 75 kg. Use 1.78 m as the provisional 1995 character-model height with the later-game temporal caveat.
3. MGSV Mother Base (1984) is not Outer Heaven (1995); use only for military-industrial materials, logistics, machinery and equipment style. MGS4 serves camera/handling quality, not period-accurate assets.
4. Newly created doors, crates and modular wall heights are modern gameplay engineering values. They are NOT historical physical measurements of 8px J2ME tiles.

## Standard metrics and geometry

Unreal units and the text OBJ model vertices are centimetres; catalogue dimensions are metres. The original-room candidate 30x24 tile grid / 8px tile art has unverified scope and unknown physical metres-per-tile. Never claim 0.9 m floor models are the original game's tile scale.

Player collision radius 0.35 m; minimum clear walkway 1.3 m; minimum clear headroom 2.4 m; camera clearance 0.35 m. These are **provisional engineering thresholds**, not universal rules. Test in engine, with both mouse and gamepad, including aiming, near walls, cover, corners and crouching.

Original authored modules in assets/greybox: floor, wall, door frame, crate, cover and equipment console. The door has an external 1.7m frame and 1.3m clear opening. OBJ bounds follow assets/asset_dimensions.json.

## Acceptance

- Verify native room grid, room ID, connections and mission flags from actual gameplay.
- Measure converted 3D room and log all intentional departures from the source tile aspect ratio.
- Validate each object's imported bounding box at 1x, with +/-1cm tolerance; confirm collision fits visual geometry.
- Confirm door traversal with 0.7m capsule and free camera; do not rely on dimensions alone.
- Run stealth and controller playtests on a packaged Windows build, including minimum-spec performance.
- Reject assets with unknown provenance, unverified mission placement, or technology that contradicts 1995.

These OBJ meshes are functional proportion placeholders, not released 3D character art or ripped models.
