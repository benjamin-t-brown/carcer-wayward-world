# Run a command in MSYS2 UCRT64 from PowerShell (used by Cursor agent and local dev).
# Usage: .\scripts\Invoke-Ucrt64.ps1 "cd src && make -j8"
#
# Resolves MSYS2 root in order:
#   1. $env:MSYS2_ROOT (explicit override)
#   2. Directory containing msys2_shell.cmd on PATH
#   3. Common install locations (official default first)
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$Command
)

function Test-Msys2Root {
    param([string]$Root)
    if (-not $Root) { return $false }
    return Test-Path -LiteralPath (Join-Path $Root 'msys2_shell.cmd')
}

function Resolve-Msys2Root {
    if (Test-Msys2Root $env:MSYS2_ROOT) {
        return $env:MSYS2_ROOT.TrimEnd('\', '/')
    }

    $shellOnPath = Get-Command msys2_shell.cmd -ErrorAction SilentlyContinue
    if ($shellOnPath) {
        return (Split-Path -Parent $shellOnPath.Source)
    }

    $candidates = @(
        'C:\msys64',                                    # official installer default
        'C:\msys2',
        'C:\progs\msys2',                               # historical project default
        'C:\tools\msys64',                              # Chocolatey-style
        (Join-Path $env:USERPROFILE 'msys64'),
        (Join-Path $env:USERPROFILE 'msys2'),
        (Join-Path $env:USERPROFILE 'scoop\apps\msys2\current')
    )

    foreach ($candidate in $candidates) {
        if (Test-Msys2Root $candidate) {
            return $candidate.TrimEnd('\', '/')
        }
    }

    return $null
}

$Msys2Root = Resolve-Msys2Root
if (-not $Msys2Root) {
    Write-Error @"
MSYS2 not found. Install MSYS2 (https://www.msys2.org/) or set MSYS2_ROOT to your install directory.

Searched: MSYS2_ROOT, PATH (msys2_shell.cmd), C:\msys64, C:\msys2, C:\progs\msys2, C:\tools\msys64, ~\msys64, ~\msys2, ~\scoop\apps\msys2\current
"@
    exit 1
}

# Expose for child processes / same-session tools (e.g. gdb path helpers).
$env:MSYS2_ROOT = $Msys2Root

$ShellCmd = Join-Path $Msys2Root 'msys2_shell.cmd'
cmd /c "`"$ShellCmd`" -ucrt64 -defterm -here -no-start -c `"$Command`""
exit $LASTEXITCODE
