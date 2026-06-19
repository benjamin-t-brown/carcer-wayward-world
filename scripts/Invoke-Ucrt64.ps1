# Run a command in MSYS2 UCRT64 from PowerShell (used by Cursor agent and local dev).
# Usage: .\scripts\Invoke-Ucrt64.ps1 "cd src && make -j8"
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$Command
)

$Msys2Root = if ($env:MSYS2_ROOT) { $env:MSYS2_ROOT } else { 'C:\progs\msys2' }
$ShellCmd = Join-Path $Msys2Root 'msys2_shell.cmd'

if (-not (Test-Path $ShellCmd)) {
    Write-Error "MSYS2 not found at $Msys2Root. Install MSYS2 or set MSYS2_ROOT."
    exit 1
}

cmd /c "`"$ShellCmd`" -ucrt64 -defterm -here -no-start -c `"$Command`""
exit $LASTEXITCODE
