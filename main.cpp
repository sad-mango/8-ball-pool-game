// =============================================================
//  main.cpp  -  8 Ball Pool | MVC Edition
//
//  Model-View-Controller Architecture:
//
//  GameModel      ->  All game state (balls, players, physics)
//  GameView       ->  All rendering (reads model, never writes)
//  GameController ->  All input + physics (writes to model)
//
//  The game loop is intentionally minimal:
//      controller.Update(model)  <- process input + physics
//      view.Render(model)        <- draw current state
// =============================================================

#include <raylib.h>
#include "config.h"
#include "mvc/GameModel.h"
#include "mvc/GameView.h"
#include "mvc/GameController.h"

int main(void) {
    InitWindow(TABLE_WIDTH, TABLE_HEIGHT + 100, "8 Ball Pool - MVC Edition");
    SetTargetFPS(TARGET_FPS);

    GameModel      model;       // M - owns all state
    GameView       view;        // V - renders model
    GameController controller;  // C - handles input & physics

    while (!WindowShouldClose()) {
        controller.Update(model);  // C writes to model
        view.Render(model);        // V reads from model
    }

    CloseWindow();
    return 0;
}
