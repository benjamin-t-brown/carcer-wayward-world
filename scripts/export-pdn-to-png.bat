@echo off
setlocal enabledelayedexpansion

REM Script to recursively find all .pdn files and export them as .png files
REM Uses pdn2png.exe for conversion

REM Get the script directory and project root
set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%.."
cd /d "%PROJECT_ROOT%"
set "TARGET_DIR=%CD%\src\assets\img"
set "CONVERTER=%SCRIPT_DIR%pdn2png.exe"

REM Check if converter exists
if not exist "%CONVERTER%" (
    echo Error: pdn2png.exe not found at: %CONVERTER%
    echo Please ensure pdn2png.exe is in the scripts directory.
    exit /b 1
)

REM Check if target directory exists
if not exist "%TARGET_DIR%" (
    echo Error: Directory not found: %TARGET_DIR%
    echo Please check the path and try again.
    exit /b 1
)

echo Using converter: %CONVERTER%
echo Searching for .pdn files in: %TARGET_DIR%
echo.

REM Counter for converted files
set /a CONVERTED=0
set /a FAILED=0

REM Find all .pdn files recursively and convert them
for /r "%TARGET_DIR%" %%F in (*.pdn) do (
    REM Get the directory and base name
    set "PDN_FILE=%%F"
    set "PNG_FILE=%%~dpnF.png"
    
    echo Converting: !PDN_FILE!
    echo   -^> !PNG_FILE!
    
    REM Convert using pdn2png.exe
    echo Running: "%CONVERTER%" "!PDN_FILE!" "!PNG_FILE!"
    "%CONVERTER%" "!PDN_FILE!" "!PNG_FILE!"
    if !errorlevel! equ 0 (
        echo   Success
        set /a CONVERTED+=1
    ) else (
        echo   Failed
        set /a FAILED+=1
    )
    echo.
)

echo Conversion complete!
echo Converted: %CONVERTED% files
if %FAILED% gtr 0 (
    echo Failed: %FAILED% files
)

endlocal

pause
