@echo off
cd /d "%~dp0"

echo ============================================
echo  Building 8 Ball Pool - MVC Edition
echo ============================================

C:\raylib\w64devkit\bin\g++.exe ^
    main.cpp ^
    balls/Ball.cpp ^
    -I. ^
    -IC:\raylib\raylib\src ^
    -LC:\raylib\raylib\src ^
    -std=c++17 ^
    -lraylib -lopengl32 -lgdi32 -lwinmm ^
    -o 8ballpool_MVC.exe

if %ERRORLEVEL% == 0 (
    echo Build successful! Launching...
    8ballpool_MVC.exe
) else (
    echo Build FAILED. Check errors above.
    pause
)
