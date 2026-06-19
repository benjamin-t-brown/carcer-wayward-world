# PowerShell wrapper for autonomous-loop summarize.sh
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$SpecDir
)

$ErrorActionPreference = "Stop"

$ScriptsDir = $PSScriptRoot
$RepoRoot = (Resolve-Path (Join-Path $ScriptsDir "../../../..")).Path
$InvokeUcrt64 = Join-Path $RepoRoot "scripts/Invoke-Ucrt64.ps1"
$BashScript = ".cursor/skills/autonomous-loop/scripts/summarize.sh"

if (Test-Path $InvokeUcrt64) {
    & $InvokeUcrt64 "$BashScript $SpecDir"
    exit $LASTEXITCODE
}

$bash = Get-Command bash -ErrorAction SilentlyContinue
if (-not $bash) {
    Write-Error "bash not found. Install MSYS2 or run via: .\scripts\Invoke-Ucrt64.ps1"
    exit 1
}

Push-Location $RepoRoot
try {
    & bash $BashScript $SpecDir
    exit $LASTEXITCODE
}
finally {
    Pop-Location
}
