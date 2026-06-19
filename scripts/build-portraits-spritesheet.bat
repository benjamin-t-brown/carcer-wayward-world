@echo off
setlocal enabledelayedexpansion

REM Build portraits0.png from port_*.png / port_*.pdn files using ImageMagick.
REM Delegates to build-portraits-spritesheet.sh (Git Bash).

set "SCRIPT_DIR=%~dp0"

where magick >nul 2>&1
if errorlevel 1 (
    echo Error: ImageMagick 'magick' not found in PATH.
    echo Install ImageMagick and ensure magick.exe is on your PATH.
    exit /b 1
)

where bash >nul 2>&1
if errorlevel 1 (
    echo Error: bash not found in PATH.
    echo Run scripts\build-portraits-spritesheet.sh from Git Bash, or add bash to PATH.
    exit /b 1
)

bash "%SCRIPT_DIR%build-portraits-spritesheet.sh" %*
exit /b %ERRORLEVEL%
