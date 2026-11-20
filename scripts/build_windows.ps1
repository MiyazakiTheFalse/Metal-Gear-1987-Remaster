# Windows package builder: requires Windows, UE5.7, VS C++ and Rust MSVC.
# Does not use paid GitHub CI. Produces a locally archived test build.
[CmdletBinding()]
param(
  [Parameter(Mandatory=$true)][string]$UnrealRoot,
  [string]$ArchiveDirectory = '',
  [switch]$ImportCharacterAssets
)
$ErrorActionPreference = 'Stop'
$Repo = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$Project = Join-Path $Repo 'unreal\MGRemaster\MGRemaster.uproject'
$Manifest = Join-Path $Repo 'rust\Cargo.toml'
$UAT = Join-Path $UnrealRoot 'Engine\Build\BatchFiles\RunUAT.bat'
if (-not (Test-Path $UAT)) { throw "Unreal AutomationTool not found: $UAT" }
if (-not (Get-Command cargo -ErrorAction SilentlyContinue)) { throw 'Rust cargo is missing from PATH' }
if (-not (Test-Path $Project)) { throw "Unreal project not found: $Project" }
if ([string]::IsNullOrWhiteSpace($ArchiveDirectory)) { $ArchiveDirectory = Join-Path $Repo 'dist\Windows' }
New-Item -ItemType Directory -Force -Path $ArchiveDirectory | Out-Null
# Game-wide Doctrine 004: refuse packaging of unregistered/incomplete beat contracts.
if (-not (Get-Command python -ErrorAction SilentlyContinue)) { throw 'Python is required for source/beat-contract checks' }
Write-Host 'Validating source and integrated experience contracts...'
& python -m unittest discover -s (Join-Path $Repo 'tools') -p 'test_*.py'
if ($LASTEXITCODE -ne 0) { throw "Source contract tests failed: $LASTEXITCODE" }
& python (Join-Path $Repo 'tools\validate_beat_contracts.py')
if ($LASTEXITCODE -ne 0) { throw "Beat contract validation failed: $LASTEXITCODE" }
& python (Join-Path $Repo 'tools\validate_area_contracts.py')
if ($LASTEXITCODE -ne 0) { throw "Area contract validation failed: $LASTEXITCODE" }
if (-not (Get-Command node -ErrorAction SilentlyContinue)) { throw 'Node.js is required to validate the browser/GLB parity inputs' }
Write-Host 'Checking browser campaign and actual rig binaries...'
& node (Join-Path $Repo 'browser\\tests\\test_campaign.cjs')
if ($LASTEXITCODE -ne 0) { throw "Browser campaign tests failed: $LASTEXITCODE" }
& node (Join-Path $Repo 'browser\\tests\\test_field_rigs.cjs')
if ($LASTEXITCODE -ne 0) { throw "Rig geometry tests failed: $LASTEXITCODE" }
& node (Join-Path $Repo 'browser\\tests\\test_animation_intent.cjs')
if ($LASTEXITCODE -ne 0) { throw "Cast animation intent tests failed: $LASTEXITCODE" }
& node (Join-Path $Repo 'browser\\tools\\audit_field_glbs.cjs')
if ($LASTEXITCODE -ne 0) { throw "Committed GLB binary audit failed: $LASTEXITCODE" }
Write-Host 'Verifying embedded Unreal Content GLB source assets...'
& python (Join-Path $Repo 'tools\verify_unreal_source_assets.py')
if ($LASTEXITCODE -ne 0) { throw 'Unreal source asset validation failed' }
Write-Host 'Building Rust gameplay core...'
& cargo build --release --manifest-path $Manifest
if ($LASTEXITCODE -ne 0) { throw "Rust build failed: $LASTEXITCODE" }
# Fail closed on missing .uassets. The checked-in GLBs are source materials:
# they are NOT native Unreal skeletal meshes until Unreal Editor imports them.
if ($ImportCharacterAssets) {
  Write-Host 'Importing native skeletal character assets with Unreal Editor...'
  & (Join-Path $PSScriptRoot 'import_unreal_content.ps1') -UnrealRoot $UnrealRoot
  if ($LASTEXITCODE -ne 0) { throw 'Unreal character import failed' }
}
$RequiredMeshes = @(
  'Snake\SK_Snake_1995_Infiltration.uasset',
  'GrayFox\SK_GrayFox_1995_Prisoner.uasset',
  'Guard\SK_Guard_1995.uasset',
  'Bosses\Shotmaker\SK_Shotmaker_1995.uasset',
  'Bosses\MachinegunKid\SK_MachinegunKid_1995.uasset'
)
foreach ($Relative in $RequiredMeshes) {
  $Native = Join-Path (Join-Path $Repo 'unreal\MGRemaster\Content\Characters') $Relative
  if (-not (Test-Path $Native)) {
    throw "Missing native Unreal SkeletalMesh: $Native. Import the committed GLBs in UE5.7 first (scripts\import_unreal_content.ps1), or use -ImportCharacterAssets after compiling the Editor project."
  }
}

# Native source-clip smoke gate: the earlier project could package five static
# imported skeletal meshes while rendering them forever in T-poses/Idle.
# Require the minimum clips our actual 1995 gameplay fallback calls.
$RequiredAnimations = @(
  'Snake\Animations\AS_Snake_Idle.uasset',
  'Snake\Animations\AS_Snake_Walk.uasset',
  'Snake\Animations\AS_Snake_Run.uasset',
  'Snake\Animations\AS_Snake_Crouch.uasset',
  'Snake\Animations\AS_Snake_Prone.uasset',
  'Snake\Animations\AS_Snake_Aim.uasset',
  'Snake\Animations\AS_Snake_Down.uasset',
  'GrayFox\Animations\AS_GrayFox_Captive.uasset',
  'GrayFox\Animations\AS_GrayFox_IdleWary.uasset',
  'Guard\Animations\AS_Guard_Walk.uasset',
  'Guard\Animations\AS_Guard_IdleAlert.uasset',
  'Guard\Animations\AS_Guard_Down.uasset',
  'Bosses\Shotmaker\Animations\AS_Shotmaker_Shoulder.uasset',
  'Bosses\Shotmaker\Animations\AS_Shotmaker_Recoil.uasset',
  'Bosses\MachinegunKid\Animations\AS_MachinegunKid_Aim.uasset',
  'Bosses\MachinegunKid\Animations\AS_MachinegunKid_Reaching.uasset'
)
foreach ($Relative in $RequiredAnimations) {
  $Native = Join-Path (Join-Path $Repo 'unreal\MGRemaster\Content\Characters') $Relative
  if (-not (Test-Path $Native)) {
    throw "Missing native gameplay AnimationSequence: $Native. Import skeletal GLBs with clips in UE5.7; do not package an idle-only cast."
  }
}
Write-Host 'Cooking, staging and packaging Unreal...'
$UatArgs = @('BuildCookRun', "-project=$Project", '-noP4', '-platform=Win64', '-clientconfig=Development', '-build', '-cook', '-stage', '-pak', '-archive', '-prereqs', "-archivedirectory=$ArchiveDirectory")
& $UAT @UatArgs
if ($LASTEXITCODE -ne 0) { throw "Unreal packaging failed: $LASTEXITCODE" }
Write-Host "Build archive: $ArchiveDirectory. Smoke-test before sharing."
