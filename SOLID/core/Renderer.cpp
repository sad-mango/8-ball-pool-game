#include "Renderer.h"
#include "Game.h"
#include <cstdio>
#include <string>
#include <cmath>

void Renderer::Draw(Game* game) {
    BeginDrawing();
    ClearBackground({ 8, 80, 23, 255 });

    DrawTable();
    DrawPockets();
    DrawBalls(game);
    DrawCueStick(game);
    DrawPowerBar(game);
    DrawHUD(game);
    DrawOverlays(game);

    EndDrawing();
}

void Renderer::DrawTable() {
    DrawRectangle(RAIL_WIDTH, RAIL_WIDTH,
                  TABLE_WIDTH  - 2 * RAIL_WIDTH,
                  TABLE_HEIGHT - 2 * RAIL_WIDTH, GREEN);
    DrawRectangle(0, 0, TABLE_WIDTH, RAIL_WIDTH, BROWN);
    DrawRectangle(0, TABLE_HEIGHT - RAIL_WIDTH, TABLE_WIDTH, RAIL_WIDTH, BROWN);
    DrawRectangle(0, 0, RAIL_WIDTH, TABLE_HEIGHT, BROWN);
    DrawRectangle(TABLE_WIDTH - RAIL_WIDTH, 0, RAIL_WIDTH, TABLE_HEIGHT, BROWN);
}

void Renderer::DrawPockets() {
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

void Renderer::DrawBalls(Game* game) {
    for (int i = 0; i < MAX_BALLS; i++) {
        Ball* b = game->ballManager.balls[i];
        if (b && !b->pocketed)
            b->Draw();
    }
}

void Renderer::DrawCueStick(Game* game) {
    if (game->ballsMoving) return;
    if (game->state != GameState::START && game->state != GameState::PLAYING) return;

    Ball*   cue        = game->ballManager.GetCueBall();
    Vector2 cueBallPos = (cue && !cue->pocketed) ? cue->position : game->cueBallPos;
    Vector2 mousePos   = GetMousePosition();

    Vector2 dir = { mousePos.x - cueBallPos.x, mousePos.y - cueBallPos.y };
    float len = sqrtf(dir.x*dir.x + dir.y*dir.y);
    if (len > 0.0001f) { dir.x /= len; dir.y /= len; }

    float effectiveLength = game->stickLength + game->stickPullPixels;
    Vector2 stickTip  = { cueBallPos.x - dir.x * (BALL_RADIUS + effectiveLength),
                          cueBallPos.y - dir.y * (BALL_RADIUS + effectiveLength) };
    Vector2 stickBase = { cueBallPos.x - dir.x * (BALL_RADIUS + 4),
                          cueBallPos.y - dir.y * (BALL_RADIUS + 4) };

    DrawLineEx(stickTip, stickBase, 8.0f, { 100, 60, 20, 255 });
    DrawLineEx(stickTip, stickBase, 6.0f, BROWN);
    DrawCircleV({ stickTip.x + dir.x * 6, stickTip.y + dir.y * 6 }, 4, LIGHTGRAY);

    if (game->aiming) {
        Vector2 lineEnd = { cueBallPos.x + dir.x * 420, cueBallPos.y + dir.y * 420 };
        DrawLineEx(cueBallPos, lineEnd, 1.5f, Fade(WHITE, 0.22f));
    }
}

void Renderer::DrawPowerBar(Game* game) {
    int x = 18, y = TABLE_HEIGHT + 70, w = 240, h = 16;

    DrawText("Power:", x, TABLE_HEIGHT + 36, 16, WHITE);
    DrawRectangle(x + 80, y, w, h, GRAY);

    int filled = (int)(w * (game->stickPullPixels / MAX_POWER_PIXELS));
    if (filled < 0) filled = 0;
    if (filled > w) filled = w;
    DrawRectangle(x + 80, y, filled, h, RED);
    DrawRectangleLines(x + 80, y, w, h, BLACK);

    char pstr[32];
    std::snprintf(pstr, sizeof(pstr), "%d%%",
                  (int)((game->stickPullPixels / MAX_POWER_PIXELS) * 100.0f));
    DrawText(pstr, x + 80 + w + 8, y - 2, 16, WHITE);
}

void Renderer::DrawHUD(Game* game) {
    DrawRectangle(0, TABLE_HEIGHT, TABLE_WIDTH, 100, { 30, 18, 10, 255 });

    for (int i = 0; i < 2; i++) {
        std::string s = game->turnManager.players[i].name + ": " +
                        std::to_string(game->turnManager.players[i].ballsRemaining) +
                        " balls remaining";
        DrawText(s.c_str(), 18, TABLE_HEIGHT + 12 + (i * 28), 18, WHITE);
    }

    PlayerType  pt         = game->turnManager.Current().type;
    std::string playerText = "Current: " + game->turnManager.Current().name;
    if      (pt == PlayerType::SOLIDS)  playerText += " (Solids)";
    else if (pt == PlayerType::STRIPES) playerText += " (Stripes)";
    else                                playerText += " (Unassigned)";

    DrawText(playerText.c_str(),          TABLE_WIDTH - 360, TABLE_HEIGHT + 12, 18, WHITE);
    DrawText(game->statusMessage.c_str(), TABLE_WIDTH - 360, TABLE_HEIGHT + 40, 16, YELLOW);
}

void Renderer::DrawOverlays(Game* game) {
    if (game->state == GameState::SCRATCH) {
        DrawRectangle(0, 0, TABLE_WIDTH, TABLE_HEIGHT + 100, { 0, 0, 0, 150 });
        const char* msg = "SCRATCH! Click to place cue ball (inside rails)";
        DrawText(msg, TABLE_WIDTH/2 - MeasureText(msg, 20)/2, TABLE_HEIGHT/2 - 10, 20, RED);
    }

    if (game->state == GameState::WON || game->state == GameState::LOST) {
        DrawRectangle(0, 0, TABLE_WIDTH, TABLE_HEIGHT + 100, { 0, 0, 0, 200 });
        std::string winText;
        if (game->state == GameState::WON)
            winText = game->turnManager.Current().name + " WINS!";
        else
            winText = game->turnManager.Other().name + " WINS!";

        DrawText(winText.c_str(),
                 TABLE_WIDTH/2 - MeasureText(winText.c_str(), 40)/2,
                 TABLE_HEIGHT/2 - 40, 40, GREEN);
        DrawText("Press R to Restart",
                 TABLE_WIDTH/2 - MeasureText("Press R to Restart", 20)/2,
                 TABLE_HEIGHT/2 + 10, 20, WHITE);
    }
}
