@echo off
echo Compiling emulator...

:: Explicitly point g++ to the MSYS2 UCRT64 include and lib directories
g++ main.cpp -o emulator.exe ^
    -IC:/msys64/ucrt64/include/SDL2 ^
    -LC:/msys64/ucrt64/lib ^
    -lmingw32 -lSDL2main -lSDL2

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Compilation failed! Check the errors above.
) else (
    echo.
    echo Compilation finished successfully! Starting emulator.exe...
    emulator.exe
)

pause
