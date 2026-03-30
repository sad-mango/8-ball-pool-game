@echo off
cd /d "%~dp0"
 
echo Building 8 Ball Pool...
 
C:\raylib\w64devkit\bin\g++.exe ^
    main.cpp ^
    balls/Ball.cpp ^
    core/PhysicsEngine.cpp ^
    core/Renderer.cpp ^
    core/InputHandler.cpp ^
    -I. ^
    -IC:\raylib\raylib\src ^
    -LC:\raylib\raylib\src ^
    -std=c++17 ^
    -lraylib -lopengl32 -lgdi32 -lwinmm ^
    -o 8ballpool.exe
 
if %ERRORLEVEL% == 0 (
    echo Build successful! Launching...
    8ballpool.exe
) else (
    echo Build FAILED.
    pause
)
 