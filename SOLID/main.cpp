#include <raylib.h>
#include "config.h"
#include "core/Game.h"
#include "core/PhysicsEngine.h"
#include "core/Renderer.h"
#include "core/InputHandler.h"

int main(void) {
    InitWindow(TABLE_WIDTH, TABLE_HEIGHT + 100, WINDOW_TITLE);
    SetTargetFPS(TARGET_FPS);

    PhysicsEngine physics;
    Renderer      renderer;
    InputHandler  input;

    Game game(&physics, &renderer, &input);

    while (!WindowShouldClose()) {
        game.Update();
        game.Draw();
    }

    CloseWindow();
    return 0;
}
