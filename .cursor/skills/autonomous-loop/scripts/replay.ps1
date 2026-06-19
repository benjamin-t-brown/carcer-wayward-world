# PowerShell wrapper for autonomous-loop replay.sh
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$SpecDir,

    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$Remaining
)

$ErrorActionPreference = "Stop"

$ScriptsDir = $PSScriptRoot
$RepoRoot = (Resolve-Path (Join-Path $ScriptsDir "../../../..")).Path
$InvokeUcrt64 = Join-Path $RepoRoot "scripts/Invoke-Ucrt64.ps1"
$BashScript = ".cursor/skills/autonomous-loop/scripts/replay.sh"

$escaped = @($SpecDir) + $Remaining | ForEach-Object {
    if ($_ -match '\s') { "'$_'" } else { $_ }
}
$bashArgs = $escaped -join " "

if (Test-Path $InvokeUcrt64) {
    & $InvokeUcrt64 "$BashScript $bashArgs"
    exit $LASTEXITCODE
}

$bash = Get-Command bash -ErrorAction SilentlyContinue
if (-not $bash) {
    Write-Error "bash not found. Install MSYS2 or run via: .\scripts\Invoke-Ucrt64.ps1"
    exit 1
}

Push-Location $RepoRoot
try {
    & bash $BashScript @($SpecDir) + $Remaining
    exit $LASTEXITCODE
}
finally {
    Pop-Location
}
