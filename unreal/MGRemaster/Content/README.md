# Unreal Content — checked-in source assets vs native Unreal assets

The Unreal project now includes **actual original skinned binary source files** (not just importer scripts):

```
unreal/MGRemaster/Content/
├── Python/
│   ├── import_field_character_glbs.py
│   └── ... other editor import scripts
└── SourceAssets/
    └── Characters/
        ├── snake.glb
        ├── fox.glb
        ├── guard.glb
        └── shoot_gunner.glb
```

These GLBs contain authored 18-joint body + jaw/brow skeletal skins, vertex colors and 14–18 animation clips. They are copied as **exact Git blobs** from the corresponding browser model files and remain in Git under the Unreal project, so this source package is self-contained. No original/proprietary MGS game assets are stored here.

**This directory does NOT yet contain native `.uasset` files.** Unreal Engine itself must be used to import the GLBs as `USkeletalMesh` / `USkeleton` / `UAnimSequence` assets. Native Unreal asset files cannot be fabricated by adding a file with a `.uasset` suffix, nor does embedding a GLB cause the engine to compile/import it automatically at runtime.

### Tester setup and generation

1. Clone `feature/character-art-pipeline-1995` (PR #3). The project is configured for **Unreal Engine 5.7** and enables `PythonScriptPlugin`.
2. Run `python tools/verify_unreal_source_assets.py` from repo root to check physical Content source blobs and binary parity.
3. Compile the Unreal C++ Editor target on the tester's UE5.7 Windows host.
4. Run `./scripts/import_unreal_content.ps1 -UnrealRoot "C:\Program Files\Epic Games\UE_5.7"` in PowerShell. Alternatively open Unreal Editor and execute `Content/Python/import_field_character_glbs.py` via the Python script command.
5. **Inspect actual output:** Unreal Content Browser `/Game/Characters/Snake`, `GrayFox`, `Guard`, `Bosses/Shotmaker`. Check every skin, imported animation and skeleton; verify each expected path resolves to a `USkeletalMesh`. Imported static meshes alone fail the contract.
6. Verify/author Animation Blueprints or explicit runtime AnimationSequence playback, the disarmed Snake costume, hand socket offsets, scale, physics and foot contact; import alone does **not** complete this work.
7. Run `./scripts/build_windows.ps1 -UnrealRoot "C:\Program Files\Epic Games\UE_5.7"` to build/cook. That script now **fails closed** if any of the four native skeletal `.uasset` files is missing; `-ImportCharacterAssets` can invoke the Editor command-line importer first.

After importing, the native `.uasset` files reside on the tester's machine and are eligible to commit (there is no `*.uasset` global Git ignore). To make GitHub itself open directly with all generated native assets, those imported files and their associated skeletons, animation sequences, materials and authored AnimBlueprints must be committed by someone **running Unreal 5.7**. Until that happens, this remains a **self-contained source project, not a fully native-asset-complete Unreal project**.

### Production reality

The currently committed character models are original **procedural prototypes**, **not** realistic MGS4-quality 1995 characters. Their inclusion is needed for independent build iteration, not evidence of production-ready faces, full skin/hair PBR, final animations, lip sync, or finished art. The full game campaign and later Outer Heaven locations are likewise not complete. Do not mark those scopes complete solely because character source assets are present.
