# Windows package builder: requires Windows, UE5.7, VS C++ and Rust MSVC.
# Does not use paid GitHub CI. Produces a locally archived test build.
[CmdletBinding()]
param(
  [Parameter(Mandatory=$true)][string]$UnrealRoot,
  [string]$ArchiveDirectory = ''
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
Write-Host 'Building Rust gameplay core...'
& cargo build --release --manifest-path $Manifest
if ($LASTEXITCODE -ne 0) { throw "Rust build failed: $LASTEXITCODE" }
Write-Host 'Cooking, staging and packaging Unreal...'
$UatArgs = @('BuildCookRun', "-project=$Project", '-noP4', '-platform=Win64', '-clientconfig=Development', '-build', '-cook', '-stage', '-pak', '-archive', '-prereqs', "-archivedirectory=$ArchiveDirectory")
& $UAT @UatArgs
if ($LASTEXITCODE -ne 0) { throw "Unreal packaging failed: $LASTEXITCODE" }
Write-Host "Build archive: $ArchiveDirectory. Smoke-test before sharing."
