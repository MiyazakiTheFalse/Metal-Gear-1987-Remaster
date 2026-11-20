# Unreal 5.7 hardening and troubleshooting implementation — 20 November 2026

**Branch:** `feature/character-art-pipeline-1995` · **Draft PR:** #3.
**Status:** committed UE C++ / Unreal Python / PowerShell / Python regression source only. **Neither UE5.7 compilation nor an actual Native Editor import/PIE/Windows package was performed in this authoring environment.** This is a focused failure-prevention pass, not a sign-off on gameplay, graphics or performance.

## Failure modes identified, diagnosed and changed in source

| Realistic failure mode in UE5.x | Specific engineering change | Observable acceptance criterion |
|---|---|---|
| Windows build starts but dynamic `LoadObject` finds no characters, clips or weapons because the cooker strips unreferenced assets | `Config/DefaultGame.ini` now cooks `/Game/Characters`, `/Game/Weapons`, `/Game/Feedback` and `/Game/Audio/Weapons` | Package opens with the same native character/animation inventory as editor; verify by actual packaged launch |
| Machinegun Kid inherits ordinary guard AnimBP after `Super::BeginPlay`, then changes skeleton | `TryUseBespokeSkeletalArt` clears inherited animation mode **before** swapping mesh; dedicated boss class or source clip driver takes precedence | No wrong-skeleton messages; Kid aims/bursts/reloads in PIE |
| Reopening an interrupted GLB import reports `existing SkeletalMesh` as complete even though native animation clips are missing | Importer now checks existing Skeleton and every named native `AnimSequence`; fails without overwriting artist assets | Re-run clean import is idempotent; partial import errors with specific missing paths |
| UE Python commandlet logs an exception but process returns 0, or PS5.1 treats native stderr as fatal prematurely | Importer and audit each write unique positive completion markers **only after success**; PowerShell checks exit, fail marker and affirmative marker; retains full log across PS5.1 and PS7 | Missing clip or Python traceback cannot produce a success printout |
| Imported GLB is 100× too small/large, has empty clips, or clips belong to the wrong Skeleton | New `Content/Python/audit_native_character_assets.py` checks all 22 cast entries for native `SkeletalMesh`, Skeleton, realistic prototype bounds (125–240cm), 17+ required clips and matching Skeleton, frame count and length | `scripts/verify_unreal_content.ps1` prints `MG_NATIVE_CHARACTER_AUDIT_PASS roles=22` only with valid native data |
| Designer-authored map already contains `AMGGrayFoxMission`, then GameMode spawns another | StartPlay now checks for an existing mission controller before spawning | One source of mission flags, one set of physical walls/guards, no SaveGame race |
| Guard appears unarmed yet fires and flashes from an unrelated actor location | Original visible cube pistol as fallback; source pistol and flash local coordinates share valid scale; guard shot trace falls back to physical flash location, not actor chest | On missing native weapon, guard carries a visible prototype pistol; correct muzzle/cover obstruction in PIE |
| Snake and Machinegun Kid fire from invisible or physically unrelated chest points | Snake shows the source pistol on actor capsule when the imported `hand_R` rig is absent; physical shot fallback follows visible muzzle flash. Kid's authoritative ray uses the receiver-attached flash | Film projectile initiation and cover accuracy from actual weapon/flash in PIE |
| Missing `hand_R` / custom `weapon_r` on a successfully imported guard | Guard logs an explicit warning and shows a fallback weapon rather than silent invisible fire | Log points to exact malformed actor rig; gameplay remains inspectable |
| Native assets exist but are unplayable or missing from package | Windows builder now refuses to cook unless **five active SkeletalMeshes**, **16 gameplay AnimationSequences**, **all 22 native cast assets** and the full native content audit are present | Test with intentionally removed asset; builder fails before `RunUAT BuildCookRun` |

The original browser and Unreal game states, weapon inventory transactions, boss hit rules, mission flags and original 1987 source progression remain authoritative. None of these safety changes auto-grant a key item or modify story causality.

## Tester sequence (local Windows / UE 5.7)

Pull the branch **with the checked-in GLB sources**. On a Windows machine with Unreal Engine 5.7, Visual Studio C++, Python, Node and Rust:

```powershell
git switch feature/character-art-pipeline-1995
git pull --ff-only
.\scripts\unreal_tester_preflight.ps1 -UnrealRoot "C:\Program Files\Epic Games\UE_5.7"
```

Once the Unreal Editor C++ target is compiled, import or validate the source models:

```powershell
.\scripts\import_unreal_content.ps1 -UnrealRoot "C:\Program Files\Epic Games\UE_5.7"
.\scripts\verify_unreal_content.ps1 -UnrealRoot "C:\Program Files\Epic Games\UE_5.7"
```

Run PIE and the source-built playable sequence; then create the local package:

```powershell
.\scripts\build_windows.ps1 -UnrealRoot "C:\Program Files\Epic Games\UE_5.7"
```

`build_windows.ps1` also invokes the native content verification. Imported `.uasset` files are not produced by cloning GitHub: Unreal Editor must create them. `-ImportCharacterAssets` on the builder is available after the Editor target compiles.

## When the test fails: first diagnostic to capture

- **No skinned mesh after import:** Check `dist/unreal-import-logs/unreal_character_import.log`; verify UE glTF/Interchange import supports skeletal mesh AND animation, and check output asset type. Never substitute a static mesh.
- **Existing skeletal mesh but missing `AS_*` clips:** The importer now fails intentionally. Inspect saved assets and source ownership. Repair using Editor/reimport or a clean clone; never delete/overwrite a collaborator's production model automatically.
- **Wrong-looking or giant/tiny characters:** Read `dist/unreal-import-logs/unreal_native_character_audit.log`; inspect imported centimetre height, pivot, skeleton and up-axis; do not 'fix' only the camera.
- **A character T-poses, freezes, or disappears:** Inspect mismatched Skeleton, incorrect imported clip, AnimBlueprint assignment, and gameplay-driven single-node fallback. The distinct Machinegun Kid skeleton must not inherit the normal Guard graph.
- **Looks correct in PIE but invisible in packaged build:** Check cook roots, check shipping archive and `LogUObjectGlobals: Warning: Failed to find object`; use a new package after validation, not a stale existing build.
- **Gun fires through a corner or visually from behind the hand:** Capture camera, target, visible muzzle/light and actual trace start at the same frame; fix authored weapon pivot/socket, not gameplay hit detection. Source fallback is interim, not muzzle-quality approved.
- **Duplicated world actors or broken/repeated progression:** Inspect level for one pre-placed `AMGGrayFoxMission`, ensure GameMode does not spawn another and use one stable checkpoint slot.
- **PowerShell reports success while Unreal log has an exception:** Check for the importer/audit positive completion marker. Without it the scripts now fail even when native process exit is zero.

## Regression source added

- `tools/test_unreal_character_import_recovery.py` — simulates partially imported native meshes, missing clips, skeleton absence and valid reruns without an Engine installation.
- `tools/test_unreal_native_character_audit_source.py` — fakes UE assets and rejects incorrect skeleton, empty animation, tiny scaled mesh and missing native mesh.
- `tools/test_unreal_packaging_cook_audit_contracts.py` — verifies cook roots, success-marker gates, boss skeleton reset, GameMode dedup and visible fallback guard pistol.
- `tools/test_unreal_muzzle_and_fallback_source.py` — locks shared visible flash / authoritative shot fallback on Snake, guard, Machinegun Kid and original shotgun.

These tests will be discovered by `python -m unittest discover -s tools -p 'test_*.py'` during tester preflight and local packaging. **They have been committed but not run in native Python or Unreal here**; full importer simulation remains source-level evidence until executed on a machine with the repository checked out.

## Hard blockers: still not complete

1. Native UE5.7 C++ compilation and fresh Editor import of 22 GLBs, followed by PIE performance, animation and grip video.
2. Professional original character-specific 1995 faces, anatomy, hairstyles, outfits, PBR texture maps and expressive believable facial/hand/body motion. Current rigs are 18-joint stylised authoring sources, **not MGS4-grade**.
3. Finished AnimBlueprints with blended locomotion, retargeted motion capture, stride/foot IK, recoil, hit reactions, body-drag contact and source-accurate weapon attachment/ballistics.
4. Real lighting/audio, original-source validated area geometry and the remaining Outer Heaven missions beyond the current Building 2 prototype.
5. Two real playable builds checked side by side, actual novice and veteran player feedback, no-crash multi-hour soak test, save recovery and measurable performance budget.

**Non-negotiable doctrine:** Do not call a feature production-ready merely because C++ compiles, a GLB exists, or a PowerShell gate is green. The physical, authored, rendered, interactive, stateful, audiovisual and player-tested experience must agree.
