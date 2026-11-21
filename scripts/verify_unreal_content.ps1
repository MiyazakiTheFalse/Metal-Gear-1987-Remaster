# UE5.7 native content gate. Detects wrongly scaled GLB characters and
# skeletal clips bound to a different skeleton, beyond .uasset file existence.
# Local validation only; this does not run Play In Editor or test aesthetics.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$UnrealRoot)
$ErrorActionPreference = 'Stop'
$Repo = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$Project = Join-Path $Repo 'unreal\MGRemaster\MGRemaster.uproject'
$Script = Join-Path $Repo 'unreal\MGRemaster\Content\Python\audit_native_character_assets.py'
$Editor = Join-Path $UnrealRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
if (-not (Test-Path $Project)) { throw "Unreal project absent: $Project" }
if (-not (Test-Path $Script)) { throw "Native skeletal audit absent: $Script" }
if (-not (Test-Path $Editor)) { throw "UnrealEditor-Cmd missing: $Editor" }
$LogDir = Join-Path $Repo 'dist\unreal-import-logs'
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
$LogPath = Join-Path $LogDir 'unreal_native_character_audit.log'
$EditorArgs = @(
    ('"' + $Project + '"'),
    '-run=pythonscript',
    ('-script="' + $Script + '"'),
    '-unattended',
    '-nop4',
    '-nosplash',
    '-log'
)
Write-Host 'Checking all 22 native character meshes, skeletal compatibility, animation tracks and imported scale...'
# PowerShell 5.1 can surface benign Unreal stderr as a terminating native
# error under Stop. Capture first, then evaluate BOTH exit and success marker.
$OldPreference = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
try {
    & $Editor @EditorArgs 2>&1 | Tee-Object -FilePath $LogPath
    $EditorExitCode = $LASTEXITCODE
} finally {
    $ErrorActionPreference = $OldPreference
}
if ($EditorExitCode -ne 0) {
    throw "Unreal native character audit exited $EditorExitCode. See $LogPath"
}
# UE Python commandlets can log an exception without a process failure.
# Require an affirmative marker emitted AFTER all 22 assets are validated.
$Passed = Select-String -Path $LogPath -Pattern 'MG_NATIVE_CHARACTER_AUDIT_PASS roles=22' -SimpleMatch -Quiet
$Failed = Select-String -Path $LogPath -Pattern 'MG_NATIVE_CHARACTER_AUDIT_FAIL' -SimpleMatch -Quiet
if ($Failed -or -not $Passed) {
    throw "Unreal character native audit did not pass (22 roles). See $LogPath"
}
Write-Host 'PASS: 22 native character assets, animation bindings and metre-to-centimetre scale.'
