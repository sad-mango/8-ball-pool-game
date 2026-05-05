#ifndef GAMEVIEW_H
#define GAMEVIEW_H

// =============================================================
//  GameView  -  VIEW layer of MVC
//
//  Reads ONLY from GameModel (const reference).
//  Responsible for ALL rendering: table, balls, cue stick,
//  power bar, HUD, overlays.
//  NEVER writes to GameModel.
//  All rendering code migrated from core/Renderer.cpp
// =============================================================

#include <string>
#include <cstdio>
#include <cmath>
#include <raylib.h>
#include "GameModel.h"
#include "../config.h"

class GameView {
public:
    // Called once per frame - renders everything
    void Render(const GameModel& model) {
        BeginDrawing();
        ClearBackground({ 8, 80, 23, 255 });

        DrawTable();
        DrawPockets();
        DrawBalls(model);
        DrawCueStick(model);
        DrawPowerBar(model);
        DrawHUD(model);
        DrawOverlays(model);

        EndDrawing();
    }

private:
    // Draw green felt + brown rails
    void DrawTable() {
        DrawRectangle(RAIL_WIDTH, RAIL_WIDTH,
                      TABLE_WIDTH  - 2 * RAIL_WIDTH,
                      TABLE_HEIGHT - 2 * RAIL_WIDTH, GREEN);
        DrawRectangle(0, 0, TABLE_WIDTH, RAIL_WIDTH, BROWN);
        DrawRectangle(0, TABLE_HEIGHT - RAIL_WIDTH, TABLE_WIDTH, RAIL_WIDTH, BROWN);
        DrawRectangle(0, 0, RAIL_WIDTH, TABLE_HEIGHT, BROWN);
        DrawRectangle(TABLE_WIDTH - RAIL_WIDTH, 0, RAIL_WIDTH, TABLE_HEIGHT, BROWN);
    }

    // Draw 6 pockets
    void DrawPockets() {
        Vector2 pockets[6] = {
            { RAIL_WIDTH,                RAIL_WIDTH },
            { TABLE_WIDTH * 0.5f,        RAIL_WIDTH },
            { TABLE_WIDTH - RAIL_WIDTH,  RAIL_WIDTH },
            { RAIL_WIDTH,                TABLE_HEIGHT - RAIL_WIDTH },
            { TABLE_WIDTH * 0.5f,        TABLE_HEIGHT - RAIL_WIDTH },
            { TABLE_WIDTH - RAIL_WIDTH,  TABLE_HEIGHT - RAIL_WIDTH }
        };
        for (int i = 0; i < 6; i++)
            DrawCircleV(pockets[i], POCKET_RADIUS, BLACK);
    }

    // Draw all non-pocketed balls using their own Draw()
    void DrawBalls(const GameModel& model) {
        for (int i = 0; i < MAX_BALLS; i++) {
            Ball* b = model.ballManager.balls[i];
            if (b && !b->pocketed)
                b->Draw();
        }
    }

    // Draw cue stick pointing from cue ball toward mouse
    void DrawCueStick(const GameModel& model) {
        if (model.ballsMoving) return;
        if (model.state != GameState::START && model.state != GameState::PLAYING) return;

        Ball*   cue        = model.ballManager.GetCueBall();
        Vector2 cueBallPos = (cue && !cue->pocketed) ? cue->position : model.cueBallPos;
        Vector2 mousePos   = GetMousePosition();

        Vector2 dir = { mousePos.x - cueBallPos.x, mousePos.y - cueBallPos.y };
        float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.0001f) { dir.x /= len; dir.y /= len; }

        float   effectiveLength = model.stickLength + model.stickPullPixels;
        Vector2 stickTip  = { cueBallPos.x - dir.x * (BALL_RADIUS + effectiveLength),
                               cueBallPos.y - dir.y * (BALL_RADIUS + effectiveLength) };
        Vector2 stickBase = { cueBallPos.x - dir.x * (BALL_RADIUS + 4),
                               cueBallPos.y - dir.y * (BALL_RADIUS + 4) };

        DrawLineEx(stickTip, stickBase, 8.0f, { 100, 60, 20, 255 });
        DrawLineEx(stickTip, stickBase, 6.0f, BROWN);
        DrawCircleV({ stickTip.x + dir.x * 6, stickTip.y + dir.y * 6 }, 4, LIGHTGRAY);

        if (model.aiming) {
            Vector2 lineEnd = { cueBallPos.x + dir.x * 420, cueBallPos.y + dir.y * 420 };
            DrawLineEx(cueBallPos, lineEnd, 1.5f, Fade(WHITE, 0.22f));
        }
    }

    // Draw power bar at the bottom strip
    void DrawPowerBar(const GameModel& model) {
        int x = 18, y = TABLE_HEIGHT + 70, w = 240, h = 16;

        DrawText("Power:", x, TABLE_HEIGHT + 36, 16, WHITE);
        DrawRectangle(x + 80, y, w, h, GRAY);

        int filled = (int)(w * (model.stickPullPixels / MAX_POWER_PIXELS));
        if (filled < 0) filled = 0;
        if (filled > w) filled = w;
        DrawRectangle(x + 80, y, filled, h, RED);
        DrawRectangleLines(x + 80, y, w, h, BLACK);

        char pstr[32];
        std::snprintf(pstr, sizeof(pstr), "%d%%",
                      (int)((model.stickPullPixels / MAX_POWER_PIXELS) * 100.0f));
        DrawText(pstr, x + 80 + w + 8, y - 2, 16, WHITE);
    }

    // Draw score / status HUD
    void DrawHUD(const GameModel& model) {
        DrawRectangle(0, TABLE_HEIGHT, TABLE_WIDTH, 100, { 30, 18, 10, 255 });

        for (int i = 0; i < 2; i++) {
            std::string s = model.turnManager.players[i].name + ": " +
                            std::to_string(model.turnManager.players[i].ballsRemaining) +
                            " balls remaining";
            DrawText(s.c_str(), 18, TABLE_HEIGHT + 12 + (i * 28), 18, WHITE);
        }

        PlayerType  pt         = model.turnManager.Current().type;
        std::string playerText = "Current: " + model.turnManager.Current().name;
        if      (pt == PlayerType::SOLIDS)  playerText += " (Solids)";
        else if (pt == PlayerType::STRIPES) playerText += " (Stripes)";
        else                                playerText += " (Unassigned)";

        DrawText(playerText.c_str(),           TABLE_WIDTH - 360, TABLE_HEIGHT + 12, 18, WHITE);
        DrawText(model.statusMessage.c_str(),  TABLE_WIDTH - 360, TABLE_HEIGHT + 40, 16, YELLOW);
    }

    // Draw scratch / win / lose overlays
    void DrawOverlays(const GameModel& model) {
        if (model.state == GameState::SCRATCH) {
            DrawRectangle(0, 0, TABLE_WIDTH, TABLE_HEIGHT + 100, { 0, 0, 0, 150 });
            const char* msg = "SCRATCH! Click to place cue ball (inside rails)";
            DrawText(msg,
                     TABLE_WIDTH / 2 - MeasureText(msg, 20) / 2,
                     TABLE_HEIGHT / 2 - 10, 20, RED);
        }

        if (model.state == GameState::WON || model.state == GameState::LOST) {
            DrawRectangle(0, 0, TABLE_WIDTH, TABLE_HEIGHT + 100, { 0, 0, 0, 200 });
            std::string winText;
            if (model.state == GameState::WON)
                winText = model.turnManager.Current().name + " WINS!";
            else
                winText = model.turnManager.Other().name + " WINS!";

            DrawText(winText.c_str(),
                     TABLE_WIDTH / 2 - MeasureText(winText.c_str(), 40) / 2,
                     TABLE_HEIGHT / 2 - 40, 40, GREEN);
            DrawText("Press R to Restart",
                     TABLE_WIDTH / 2 - MeasureText("Press R to Restart", 20) / 2,
                     TABLE_HEIGHT / 2 + 10, 20, WHITE);
        }
    }
};

#endif // GAMEVIEW_H
