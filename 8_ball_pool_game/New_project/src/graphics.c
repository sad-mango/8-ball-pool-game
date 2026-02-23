#include "graphics.h"

void DrawTable(void) {
    // Felt surface
    DrawRectangle(RAIL_WIDTH, RAIL_WIDTH,
                  TABLE_WIDTH  - 2*RAIL_WIDTH,
                  TABLE_HEIGHT - 2*RAIL_WIDTH, GREEN);
    // Rails
    DrawRectangle(0, 0, TABLE_WIDTH, RAIL_WIDTH, BROWN);
    DrawRectangle(0, TABLE_HEIGHT - RAIL_WIDTH, TABLE_WIDTH, RAIL_WIDTH, BROWN);
    DrawRectangle(0, 0, RAIL_WIDTH, TABLE_HEIGHT, BROWN);
    DrawRectangle(TABLE_WIDTH - RAIL_WIDTH, 0, RAIL_WIDTH, TABLE_HEIGHT, BROWN);
}

void DrawBalls(Game *game) {
    for (int i = 0; i < MAX_BALLS; i++) {
        if (game->balls[i].pocketed) continue;

        DrawCircleV(game->balls[i].position, BALL_RADIUS, game->balls[i].color);

        if (game->balls[i].isStriped) {
            DrawRectangleV(
                (Vector2){ game->balls[i].position.x - BALL_RADIUS*0.9f,
                           game->balls[i].position.y - BALL_RADIUS*0.28f },
                (Vector2){ BALL_RADIUS*1.8f, BALL_RADIUS*0.56f },
                WHITE);
            DrawCircleV(game->balls[i].position, BALL_RADIUS - 1, game->balls[i].color);
        }

        if (game->balls[i].type == BALL_CUE) {
            DrawCircleV(game->balls[i].position, 4, LIGHTGRAY);
        } else {
            char numStr[4];
            sprintf(numStr, "%d", game->balls[i].number);
            Vector2 tp = {
                game->balls[i].position.x - MeasureText(numStr, 12) / 2.0f,
                game->balls[i].position.y - 6
            };
            DrawText(numStr, (int)tp.x, (int)tp.y, 12, WHITE);
        }
    }
}

void DrawCueStick(Game *game) {
    if (game->ballsMoving) return;
    if (game->state != GAME_START && game->state != GAME_PLAYING) return;

    Vector2 cueBallPos = game->balls[0].pocketed ? game->cueBallPos : game->balls[0].position;
    Vector2 mousePos   = GetMousePosition();

    Vector2 dir = { mousePos.x - cueBallPos.x, mousePos.y - cueBallPos.y };
    float len = sqrtf(dir.x*dir.x + dir.y*dir.y);
    if (len > 0.0001f) { dir.x /= len; dir.y /= len; }

    float effectiveLength = game->stickLength + game->stickPullPixels;
    Vector2 stickTip   = { cueBallPos.x - dir.x * (BALL_RADIUS + effectiveLength),
                           cueBallPos.y - dir.y * (BALL_RADIUS + effectiveLength) };
    Vector2 stickBase  = { cueBallPos.x - dir.x * (BALL_RADIUS + 4),
                           cueBallPos.y - dir.y * (BALL_RADIUS + 4) };

    DrawLineEx(stickTip, stickBase, 8.0f, (Color){100, 60, 20, 255});
    DrawLineEx(stickTip, stickBase, 6.0f, BROWN);
    Vector2 tipPos = { stickTip.x + dir.x*6, stickTip.y + dir.y*6 };
    DrawCircleV(tipPos, 4, LIGHTGRAY);

    // Aiming line
    if (game->aiming) {
        Vector2 lineEnd = { cueBallPos.x + dir.x * 420, cueBallPos.y + dir.y * 420 };
        DrawLineEx(cueBallPos, lineEnd, 1.5f, Fade(WHITE, 0.22f));
    }
}

void DrawPowerBar(Game *game) {
    int x      = 18;
    int y      = TABLE_HEIGHT + 70;
    int width  = 240;
    int height = 16;

    DrawText("Power:", x, TABLE_HEIGHT + 36, 16, WHITE);
    DrawRectangle(x + 80, y, width, height, GRAY);

    int filled = (int)(width * (game->stickPullPixels / MAX_POWER_PIXELS));
    if (filled < 0) filled = 0;
    if (filled > width) filled = width;
    DrawRectangle(x + 80, y, filled, height, RED);
    DrawRectangleLines(x + 80, y, width, height, BLACK);

    char pstr[32];
    sprintf(pstr, "%d%%", (int)((game->stickPullPixels / MAX_POWER_PIXELS) * 100.0f));
    DrawText(pstr, x + 80 + width + 8, y - 2, 16, WHITE);
}

void DrawHUD(Game *game) {
    DrawRectangle(0, TABLE_HEIGHT, TABLE_WIDTH, 100, (Color){30, 18, 10, 255});

    char scoreText[128];
    sprintf(scoreText, "%s: %d balls remaining", game->players[0].name, game->players[0].ballsRemaining);
    DrawText(scoreText, 18, TABLE_HEIGHT + 12, 18, WHITE);
    sprintf(scoreText, "%s: %d balls remaining", game->players[1].name, game->players[1].ballsRemaining);
    DrawText(scoreText, 18, TABLE_HEIGHT + 40, 18, WHITE);

    char playerText[80];
    PlayerType pt = game->players[game->currentPlayer].type;
    if      (pt == PLAYER_SOLIDS)  sprintf(playerText, "Current: %s (Solids)",     game->players[game->currentPlayer].name);
    else if (pt == PLAYER_STRIPES) sprintf(playerText, "Current: %s (Stripes)",    game->players[game->currentPlayer].name);
    else                           sprintf(playerText, "Current: %s (Unassigned)", game->players[game->currentPlayer].name);

    DrawText(playerText, TABLE_WIDTH - 360, TABLE_HEIGHT + 12, 18, WHITE);
    DrawText(game->statusMessage, TABLE_WIDTH - 360, TABLE_HEIGHT + 40, 16, YELLOW);
}

void DrawOverlays(Game *game) {
    if (game->state == GAME_SCRATCH) {
        DrawRectangle(0, 0, TABLE_WIDTH, TABLE_HEIGHT + 100, (Color){0, 0, 0, 150});
        const char *msg = "SCRATCH! Click to place cue ball (inside rails)";
        DrawText(msg, TABLE_WIDTH/2 - MeasureText(msg, 20)/2, TABLE_HEIGHT/2 - 10, 20, RED);
    }

    if (game->state == GAME_WON || game->state == GAME_LOST) {
        DrawRectangle(0, 0, TABLE_WIDTH, TABLE_HEIGHT + 100, (Color){0, 0, 0, 200});
        char winText[64];
        if (game->state == GAME_WON) {
            sprintf(winText, "%s WINS!", game->players[game->currentPlayer].name);
        } else {
            int w = 1 - game->currentPlayer;
            sprintf(winText, "%s WINS!", game->players[w].name);
        }
        DrawText(winText, TABLE_WIDTH/2 - MeasureText(winText, 40)/2, TABLE_HEIGHT/2 - 40, 40, GREEN);
        DrawText("Press R to Restart",
                 TABLE_WIDTH/2 - MeasureText("Press R to Restart", 20)/2,
                 TABLE_HEIGHT/2 + 10, 20, WHITE);
    }
}

void DrawGame(Game *game) {
    BeginDrawing();
    ClearBackground((Color){8, 80, 23, 255});

    DrawTable();

    // Pockets
    Vector2 pockets[6];
    GetPocketPositions(pockets);
    for (int i = 0; i < 6; i++) DrawCircleV(pockets[i], POCKET_RADIUS, BLACK);

    DrawBalls(game);
    DrawCueStick(game);
    DrawPowerBar(game);
    DrawHUD(game);
    DrawOverlays(game);

    EndDrawing();
}
