# Unreal 5.7 tester handoff — development source, 20 November 2026

## First read: truthfulness and scope

**PR #3 remains draft.** Browser-source campaign tests now cover progression from infiltration, Gray Fox, Shoot Gunner and Machinegun Kid into the connected **Building 1 roof Hind D** and **Building 2 tank/mine route**. The browser harness passed 15 logged regression scenarios in the authoring session; it is **not** equivalent to opening Unreal or playing a packaged game. The new C++ encounter code has been committed but has **not** been compiled or played in Unreal. Character GLBs are 18-joint skinned **procedural prototypes, not realistic finished art**; they have not been imported to the Unreal project's Content folder in the authoring environment.

The tester must check out **`feature/character-art-pipeline-1995`**, not `main`. PR: https://github.com/MiyazakiTheFalse/Metal-Gear-1987-Remaster/pull/3

## Host prerequisites

- Windows x64 development host, enough disk space for Unreal DerivedDataCache + Win64 build/cook.
- **Unreal Engine 5.7** installed at a known path; Unreal Python Editor Script Plugin and Interchange glTF skeletal import must be enabled for the model import step.
- Visual Studio 2022 with **Desktop development with C++** and **Game development with C++**, compatible Windows SDK and MSVC toolchain. Use UE 5.7's toolchain recommendation.
- Git, Rust and the `x86_64-pc-windows-msvc` target, Cargo, Python 3 and Node.js. No paid GitHub Actions/Copilot/CI required.
- Original J2ME files and Konami proprietary assets are **not** needed to execute the original prototype source; do not put them in a public repository.

## Clone the exact current development branch

Open PowerShell:

```powershell
git clone --branch feature/character-art-pipeline-1995 --single-branch https://github.com/MiyazakiTheFalse/Metal-Gear-1987-Remaster.git
cd Metal-Gear-1987-Remaster
git pull --ff-only
git rev-parse HEAD
.\scripts\unreal_tester_preflight.ps1 -UnrealRoot "C:\Program Files\Epic Games\UE_5.7"
```

**If any preflight check fails, report the first failing command and complete output; do not proceed as if the build is verified.** The script checks Git, engine paths, 17 browser campaign checkpoints (simulated harness evidence), a new deterministic animation-intent test, four core GLB binary contracts, Python source contracts and Rust tests. It cannot run the Unreal Editor in this project-authoring session.

## Import visual character rigs before judging character quality

Start `unreal/MGRemaster/MGRemaster.uproject` in Unreal Editor 5.7. Under Python scripting use **Tools > Execute Python Script** (or the Python console's `exec(open(...).read())`) and select:

`unreal/MGRemaster/Content/Python/import_field_character_glbs.py`

The Unreal checkout now directly includes 22 source GLBs (four primary gameplay rigs plus 18 supporting humanoid cast prototypes) under `unreal/MGRemaster/Content/SourceAssets/Characters/`. They are original skinned assets, **not yet native `.uasset` files**. From the repository root, first run `python tools/verify_unreal_source_assets.py` to confirm the copies have not diverged from the browser rigs. After compiling the C++ Editor target, use `scripts/import_unreal_content.ps1 -UnrealRoot "C:\\Program Files\\Epic Games\\UE_5.7"` for command-line generation, or use the Editor script manually.

This imports original source GLBs under the exact C++ character load paths:

| Actor | Expected UE skeletal mesh asset path |
|---|---|
| Solid Snake | `/Game/Characters/Snake/SK_Snake_1995_Infiltration` |
| Gray Fox | `/Game/Characters/GrayFox/SK_GrayFox_1995_Prisoner` |
| Guard | `/Game/Characters/Guard/SK_Guard_1995` |
| Shoot Gunner | `/Game/Characters/Bosses/Shotmaker/SK_Shotmaker_1995` |

The script rejects invalid GLB headers, missing source joints, incomplete clip inventories and static-only import results. It renames an Interchange-generated skeletal mesh to the expected target only if the path is empty; **it does not overwrite existing artist work**. If editor import fails, capture Output Log and check glTF/Interchange plugin support. **Do not silently substitute static meshes.**

Verify skeletal import on each asset: 18 actual joints, skinned mesh bound to the skeleton, sensible axis and scale, correct floor contact, animate a full idle/walk/run/crouch/prone/action sequence, no severe clipping, and check a talking jaw/brow. Check that the expected animation assets are actually available in Unreal—source GLB clips alone do not prove UE AnimationSequence imports or runtime AnimBlueprint binding. With no `ABP_...`, the new source-only `MGPrototypeAnimationFallback.h` can play imported single-node clips for Snake, Gray Fox, guards, Shoot Gunner and Machinegun Kid, instead of a perpetual idle. Check actual state switching (walk/run, low stances, aim, guard search/reload, Gray Fox captive-to-rescued, Shotmaker shoulder/recoil and Kid's sight/burst/reload). The source rigs have `hand_R` but may not have custom `weapon_r` sockets: test the new hand-bone attachment fallback for Snake, guards and both armed bosses; record grip offsets, actual muzzle alignment and shell ejection in PIE. This is an **interim, unblended fallback**, not a passed production animation graph or in-editor verification. An existing valid AnimBlueprint takes precedence. If clip imports are missing, the fallback cannot animate them. Snake's separate disarmed outfit is still unmade; a mesh import cannot fabricate it.

## Compile, PIE and local Windows package

Compile the C++ project using UE 5.7's Visual Studio/UBT integration, then open **Play In Editor**. The default map is `/Engine/Maps/Entry`; `AMGRemasterGameMode` spawns the player and `AMGGrayFoxMission` scene actors. If no actors or game mode appear, inspect `DefaultEngine.ini`, GameMode/StartPlay and actor spawning before judging level design.

To produce a **locally built** Windows development package (after passing preflight):

```powershell
.\scripts\build_windows.ps1 -UnrealRoot "C:\Program Files\Epic Games\UE_5.7"
```

This runs local Python+Node contracts, verifies the GLBs under Unreal `Content/SourceAssets`, releases the Rust gameplay core, and then requires four native SkeletalMesh `.uasset` files before `RunUAT.bat BuildCookRun`. If you have not already imported the character source assets in Unreal, run the import script above first or pass `-ImportCharacterAssets` to the packaging script **after** the Editor target compiles. The script now explicitly refuses to package a purportedly complete character build with missing native meshes. Default archive is `dist\Windows`, which is ignored by Git. A *committed source project is not a prebuilt exe*. Verify startup, HUD, input and file provenance for any packaged exe before handing it to a nontechnical tester.

## Playtest progression and critical assertions

1. Open with the original operation briefing; verify third-person Snake model actually renders and moves, with controller and keyboard input. Check aiming, crouch, prone, cover and guard awareness; guard body incapacitation/dragging must remain physical and checkpoint-persistent.
2. Capture, prison breach, Gray Fox, equipment reclaim, Shoot Gunner, Card 2/3, kennels, service charge, Machinegun Kid. Watch for any dead ends, missing props, ambiguous objective prompt, impossible wall collision, broken cutscene handoff and lost inventory.
3. Defeat Machinegun Kid and collect parachute. Use the west lift to enter the elevated Building 1 roof. Locate physically solid blast shelter, observe the helicopter's warning and firing cycle, use the provisional grenade rack. Check roof withdrawal and re-entry, finite ammo and boss health/checkpoint persistence.
4. After Hind D defeat, use the roof approach transition to Building 2. Confirm the tank patrol is protected from ordinary firearm damage. Plant limited mines at **marked physical track sites**, retreat from the warning/shot and wait for an actual tank track crossing. Confirm a mine is consumed on detonation; cover breaks turret sight; depleted mines do not regenerate on save reload. Return to roof then re-enter Building 2 without resetting tank progress.
5. Restore pre-existing V1–V6 Unreal saves without impossible teleport or inventory fabrication; new V7 checkpoints include the original western-truck ration (finite remaining amount), central-truck Card 1, eastern-truck binoculars, tank/mine progress, roof access and boss completion. Test a supply-only checkpoint before handgun acquisition: reload must accept it without granting unearned weapons. Test ration consumption before and after custody/recovery: it must not respawn. The browser V10 checkpoint is a separate save format and is **not** imported into Unreal.
6. Record cold-start FPS, hitches, shader errors, weapon/character offsets, animation foot sliding, muzzle alignment, subtitle/voice parity, texture quality, build warnings and errors. Explicitly test Use Item binocular zoom across ten seconds of Tick and during crouch/aim, since the previous camera update silently overwrote it. Verify KO guards use either the imported Down sequence or the honest fallen proxy, but never both rotations. Compare screenshots in prison, boss hall, roof and Building 2 in both browser and Unreal.

## Updated first-floor 1987 acquisition checks

In each of the **three separately accessible supply trucks**, interact with the physical prop from the rear cargo entrance. After physically recovering gear, verify that the return freight shutter at the west prison boundary opens and allows genuine backtracking into the original yard; do **not** grant missing Card 1 automatically. Return to the centre truck if the player skipped it, collect it, and reload the resulting checkpoint while standing in the yard. The west grants one ration, middle grants Card 1, east grants binoculars. Confirm that the HUD provides local interaction affordance, item ownership survives custody/recovery and reload, consumed rations do not refill, and binoculars change optical FOV without revealing enemies through cover. These are authored mechanics, not yet PIE-tested. Track source changes and test contracts in [the opening implementation record](ORIGINAL_TRUCK_OPENING_IMPLEMENTATION_2026_11_20.md).

## Pass/fail expectations

**Blocking:** Unreal C++ compile failure; no player spawn; invisible character; missing required body rig when test is about character visual quality; collision/traversal softlock; objective unable to advance; tank or Hind defeat without actual counterplay; broken saves; asset import failure.

**Nonblocking but outstanding:** placeholder procedural geometry, unfinished realistic heads and PBR textures, no final mocap/blendshapes, no authored cinematic camera passes or VO for later bosses, no complete Building 2 exit or Bulldog/other bosses, and uncertain source-accuracy of provisional 3D coordinates.

**Required tester deliverable:** `git rev-parse HEAD` SHA, engine version, imported asset inventory, machine specs, logs of preflight and UAT/UBT, at least one 720p gameplay capture/screenshots per encounter, exact reproduction for every failure, whether the packaged Win64 executable actually launched, and list of pass/fail assertions. Do not merge draft PR or label production-ready from static tests alone.

## Notes about originality and asset licensing

The author-owned procedural GLBs are present in `browser/assets/models` and are source-editable via `browser/tools/export_field_rigs.cjs`. These are **not ripped Konami character models** or MGS4-quality licensed meshes. Preserve the project’s own copyright, provenance, and source/lore verification rules when replacing them. No original franchise art/VO is included merely by pulling the repo.
