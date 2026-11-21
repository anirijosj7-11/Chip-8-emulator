@echo off
setlocal enabledelayedexpansion

:: Anchor to the folder containing build.bat
cd /d "%~dp0"

echo Compiling emulator...

g++ -std=c++17 -O2 main.cpp -o emulator.exe ^
    -IC:/msys64/ucrt64/include ^
    -IC:/msys64/ucrt64/include/SDL2 ^
    -LC:/msys64/ucrt64/lib ^
    -lmingw32 -lSDL2main -lSDL2

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Compilation failed! Check the errors above.
    pause
    exit /b 1
)

echo.
echo Compilation finished successfully!

:: If a ROM path/name was passed on the command line, use it directly
set "DIRECT_ROM=%~1"
if not "%DIRECT_ROM%"=="" (
    if exist "%DIRECT_ROM%" (
        set "CHOSEN_ROM=%DIRECT_ROM%"
        goto :run_emu
    ) else if exist "roms\%DIRECT_ROM%" (
        set "CHOSEN_ROM=roms\%DIRECT_ROM%"
        goto :run_emu
    ) else if exist "roms\%DIRECT_ROM%.ch8" (
        set "CHOSEN_ROM=roms\%DIRECT_ROM%.ch8"
        goto :run_emu
    )
)

:: Check that the roms folder exists
if not exist "roms\" (
    echo [ERROR] Folder "roms" not found at "%~dp0roms".
    pause
    exit /b 1
)

:: Collect files inside roms\ into an indexed list
set /a COUNT=0
echo.
echo Available ROMs in "roms\":
echo -----------------------------------------
for %%F in ("roms\*") do (
    if not "%%~xF"=="" (
        set /a COUNT+=1
        set "ROM_!COUNT!=%%~fF"
        set "NAME_!COUNT!=%%~nxF"
        echo   [!COUNT!] %%~nxF
    )
)
echo -----------------------------------------

if !COUNT! equ 0 (
    echo [ERROR] No ROM files found inside "%~dp0roms".
    pause
    exit /b 1
)

:ask_choice
set "CHOICE="
set /p "CHOICE=Enter ROM number (1-!COUNT!): "

if "!CHOICE!"=="" goto :ask_choice

:: Validate numeric choice in range 1..COUNT
set "VALID=0"
for /L %%N in (1,1,!COUNT!) do (
    if "!CHOICE!"=="%%N" (
        set "CHOSEN_ROM=!ROM_%%N!"
        set "CHOSEN_NAME=!NAME_%%N!"
        set "VALID=1"
    )
)

if "!VALID!"=="0" (
    echo [WARN] Invalid selection. Please enter a number between 1 and !COUNT!.
    goto :ask_choice
)

:run_emu
echo.
echo Starting emulator.exe with "!CHOSEN_ROM!"...
emulator.exe "!CHOSEN_ROM!"

pause