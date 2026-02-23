#include "common.h"
#include "game.h"
#include "graphics.h"

int main(void) {
    InitWindow(TABLE_WIDTH, TABLE_HEIGHT + 100, WINDOW_TITLE);
    SetTargetFPS(TARGET_FPS);

    Game game;
    InitGame(&game);

    while (!WindowShouldClose()) {
        UpdateGame(&game);
        DrawGame(&game);
    }

    CloseWindow();
    return 0;
}
