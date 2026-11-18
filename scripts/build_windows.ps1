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
Write-Host 'Building Rust gameplay core...'
& cargo build --release --manifest-path $Manifest
if ($LASTEXITCODE -ne 0) { throw "Rust build failed: $LASTEXITCODE" }
Write-Host 'Cooking, staging and packaging Unreal...'
$Args = @('BuildCookRun', "-project=$Project", '-noP4', '-platform=Win64', '-clientconfig=Development', '-build', '-cook', '-stage', '-pak', '-archive', '-prereqs', "-archivedirectory=$ArchiveDirectory")
& $UAT @Args
if ($LASTEXITCODE -ne 0) { throw "Unreal packaging failed: $LASTEXITCODE" }
Write-Host "Build archive: $ArchiveDirectory. Smoke-test before sharing."
