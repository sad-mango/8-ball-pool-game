@echo off
echo Building 8-Ball Pool...

set RAYLIB=C:\raylib\raylib\src

gcc -std=c99 -O2 src/main.c src/game.c src/graphics.c src/physics.c src/utils.c ^
    -I./include -I%RAYLIB% ^
    -L%RAYLIB% -lraylib -lopengl32 -lgdi32 -lwinmm ^
    -o 8ball_pool.exe

if %ERRORLEVEL% == 0 (
    echo.
    echo Build successful! Running game...
    8ball_pool.exe
) else (
    echo.
    echo Build failed! Make sure raylib is installed at C:\raylib
    echo Download from: https://github.com/raysan5/raylib/releases
    pause
)