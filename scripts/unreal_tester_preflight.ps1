# Offline preflight for the Unreal Windows tester (no paid CI).
# Run from PowerShell: .\scripts\unreal_tester_preflight.ps1 -UnrealRoot "C:\Program Files\Epic Games\UE_5.7"
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$UnrealRoot)
$ErrorActionPreference = 'Stop'
$Repo = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$Project = Join-Path $Repo 'unreal\MGRemaster\MGRemaster.uproject'
if (-not (Test-Path $Project)) { throw "Missing project: $Project" }
$Version = (Get-Content $Project -Raw | ConvertFrom-Json).EngineAssociation
if ($Version -ne '5.7') { throw "Unexpected UE project version: $Version; expected 5.7" }
$Editor = Join-Path $UnrealRoot 'Engine\Binaries\Win64\UnrealEditor.exe'
$Uat = Join-Path $UnrealRoot 'Engine\Build\BatchFiles\RunUAT.bat'
if (-not (Test-Path $Editor)) { throw "Missing UnrealEditor executable: $Editor" }
if (-not (Test-Path $Uat)) { throw "Missing UAT build tool: $Uat" }
foreach ($name in @('node','python','cargo','git')) {
  if (-not (Get-Command $name -ErrorAction SilentlyContinue)) { throw "Missing executable on PATH: $name" }
}
$Branch = & git -C $Repo rev-parse --abbrev-ref HEAD
$Commit = & git -C $Repo rev-parse HEAD
if ($LASTEXITCODE -ne 0) { throw 'Could not determine Git commit' }
Write-Host "Repo: $Repo"
Write-Host "Branch: $Branch"
Write-Host "Commit: $Commit"
if ($Branch -ne 'feature/character-art-pipeline-1995') {
  Write-Warning 'This checkout is not the current Unreal-tester development branch. Verify the commit in PR #3.'
}
$Changes = & git -C $Repo status --porcelain
if ($Changes) { Write-Warning "Working tree has local changes; preserve your tester edits: $Changes" }
foreach ($role in @('snake','fox','guard','shoot_gunner')) {
  $Asset = Join-Path $Repo "unreal\MGRemaster\Content\SourceAssets\Characters\$role.glb"
  if (-not (Test-Path $Asset)) { throw "Missing committed GLB: $Asset" }
  if ((Get-Item $Asset).Length -lt 100000) { throw "Suspiciously small GLB: $Asset" }
}
function Check-Exit([string]$Label) {
  if ($LASTEXITCODE -ne 0) { throw "$Label failed with exit code $LASTEXITCODE" }
}
Write-Host 'Validating self-contained Unreal content sources and their binary parity...'
& python (Join-Path $Repo 'tools\verify_unreal_source_assets.py')
Check-Exit 'Unreal source asset manifest'
Write-Host "Running browser campaign contract..."
& node (Join-Path $Repo 'browser\tests\test_campaign.cjs')
Check-Exit 'browser campaign'
Write-Host 'Running full-body/face rig geometry checks...'
& node (Join-Path $Repo 'browser\tests\test_field_rigs.cjs')
Check-Exit 'rig geometry'
Write-Host 'Running gameplay-owned character animation-selection checks...'
& node (Join-Path $Repo 'browser\tests\test_animation_intent.cjs')
Check-Exit 'cast animation intent'
Write-Host 'Verifying the actual committed binary GLBs and animation clips...'
& node (Join-Path $Repo 'browser\tools\audit_field_glbs.cjs')
Check-Exit 'GLB audit'
Write-Host 'Running Python source/area/save/campaign contracts...'
& python -m unittest discover -s (Join-Path $Repo 'tools') -p 'test_*.py'
Check-Exit 'Python source contracts'
Write-Host 'Running native Rust core tests...'
& cargo test --manifest-path (Join-Path $Repo 'rust\Cargo.toml')
Check-Exit 'Rust core tests'
Write-Host 'PASS: source preflight and local test prerequisites.'
Write-Warning 'This does NOT verify an Unreal compile or PIE run. Next: import skinned GLBs into Editor, inspect skeletal assets and animation, then run build_windows.ps1 to package.'
