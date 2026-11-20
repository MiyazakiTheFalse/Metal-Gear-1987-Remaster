# Rebuild native Unreal asset cache from the committed Content/SourceAssets GLBs.
# Requires Windows and Unreal Engine 5.7. No paid CI or external art downloads.
# Usage: .\scripts\import_unreal_content.ps1 -UnrealRoot "C:\Program Files\Epic Games\UE_5.7"
[CmdletBinding()]
param([Parameter(Mandatory = $true)][string]$UnrealRoot)
$ErrorActionPreference = 'Stop'
$Repo = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$Project = Join-Path $Repo 'unreal\MGRemaster\MGRemaster.uproject'
$Importer = Join-Path $Repo 'unreal\MGRemaster\Content\Python\import_field_character_glbs.py'
$Editor = Join-Path $UnrealRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
if (-not (Test-Path $Editor)) { throw "UnrealEditor-Cmd.exe missing: $Editor" }
if (-not (Test-Path $Project)) { throw "Project missing: $Project" }
if (-not (Test-Path $Importer)) { throw "Importer missing: $Importer" }
if (-not (Get-Command python -ErrorAction SilentlyContinue)) { throw 'Python 3 is required' }
& python (Join-Path $Repo 'tools\verify_unreal_source_assets.py')
if ($LASTEXITCODE -ne 0) { throw 'Unreal source GLB validation failed before editor import.' }

$Logs = Join-Path $Repo 'dist\unreal-import-logs'
New-Item -ItemType Directory -Force -Path $Logs | Out-Null
$EditorArgs = @(
    ('"' + $Project + '"'),
    '-run=pythonscript',
    ('-script="' + $Importer + '"'),
    '-unattended',
    '-nop4',
    '-nosplash',
    '-log'
)
Write-Host 'Generating native SkeletalMesh and available AnimationSequence assets from committed GLBs...'
& $Editor @EditorArgs 2>&1 | Tee-Object -FilePath (Join-Path $Logs 'unreal_character_import.log')
if ($LASTEXITCODE -ne 0) {
    throw "Unreal character import FAILED ($LASTEXITCODE). Inspect the import log."
}
Write-Host 'Editor import finished. Inspect imported Skeletons/AnimationSequences and configure runtime AnimBlueprints before gameplay QA.'
Write-Warning 'Native .uasset files are only on this machine until they are independently checked into Git.'
