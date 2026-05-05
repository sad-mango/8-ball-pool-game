#include "game.h"
#include "physics.h"
#include "utils.h"

void InitGame(Game *game) {
    strcpy(game->players[0].name, "Player 1");
    game->players[0].type = PLAYER_NONE;
    game->players[0].ballsRemaining = 7;

    strcpy(game->players[1].name, "Player 2");
    game->players[1].type = PLAYER_NONE;
    game->players[1].ballsRemaining = 7;

    game->currentPlayer = 0;
    game->state = GAME_START;
    game->power = 0.0f;
    game->aiming = false;
    game->ballsMoving = false;
    game->firstShot = true;
    game->assignedTypes = false;
    strcpy(game->statusMessage, "Break shot: click on cue, drag back, release to shoot");

    game->stickPullPixels = 0.0f;
    game->stickLength = STICK_LENGTH;
    game->stickRecoil = false;
    game->recoilTimer = 0.0f;

    ResetBalls(game);
}

void ResetBalls(Game *game) {
    Vector2 triangleStart = { TABLE_WIDTH * 0.72f, TABLE_HEIGHT * 0.5f };

    // Cue ball
    game->balls[0].position = (Vector2){ TABLE_WIDTH * 0.25f, TABLE_HEIGHT * 0.5f };
    game->balls[0].velocity  = (Vector2){0, 0};
    game->balls[0].color     = WHITE;
    game->balls[0].type      = BALL_CUE;
    game->balls[0].number    = 0;
    game->balls[0].pocketed  = false;
    game->balls[0].isStriped = false;

    Color solidColors[]  = { YELLOW, BLUE, RED, PURPLE, ORANGE, SKYBLUE, MAROON };
    Color stripeColors[] = { YELLOW, BLUE, RED, PURPLE, ORANGE, SKYBLUE, MAROON };

    int idx = 1;
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col <= row; col++) {
            if (idx >= MAX_BALLS) break;
            float offsetX = row * (BALL_RADIUS * 2 * 0.88f);
            float offsetY = (col * (BALL_RADIUS * 2)) - (row * BALL_RADIUS);
            game->balls[idx].position = (Vector2){ triangleStart.x + offsetX, triangleStart.y + offsetY };
            game->balls[idx].velocity = (Vector2){0, 0};
            game->balls[idx].pocketed = false;

            if (idx == 8) {
                game->balls[idx].color    = BLACK;
                game->balls[idx].type     = BALL_EIGHT;
                game->balls[idx].isStriped = false;
            } else if (idx <= 7) {
                game->balls[idx].color    = solidColors[idx - 1];
                game->balls[idx].type     = BALL_SOLID;
                game->balls[idx].isStriped = false;
            } else {
                int sidx = idx - 9;
                if (sidx < 0) sidx = 0;
                game->balls[idx].color    = stripeColors[sidx];
                game->balls[idx].type     = BALL_STRIPE;
                game->balls[idx].isStriped = true;
            }
            game->balls[idx].number = idx;
            idx++;
        }
    }

    game->cueBallPos = game->balls[0].position;
}

void UpdateGame(Game *game) {
    HandleInput(game);

    // Stick recoil animation
    if (game->stickRecoil) {
        game->recoilTimer -= 1.0f / TARGET_FPS;
        if (game->recoilTimer <= 0.0f) {
            game->stickRecoil = false;
            game->stickPullPixels = 0.0f;
        } else {
            game->stickPullPixels *= 0.92f;
            game->power = game->stickPullPixels / MAX_POWER_PIXELS;
            if (game->power < 0) game->power = 0;
        }
    }

    if (game->state == GAME_PLAYING || game->state == GAME_SCRATCH) {
        UpdatePhysics(game);

        if (!game->ballsMoving && AreBallsMoving(game)) game->ballsMoving = true;

        if (game->ballsMoving && !AreBallsMoving(game)) {
            game->ballsMoving = false;
            if (game->state == GAME_PLAYING) {
                CheckWinCondition(game);
                if (game->state != GAME_WON && game->state != GAME_LOST) {
                    NextTurn(game);
                }
            }
        }
    }
}

void HandleInput(Game *game) {
    if (IsKeyPressed(KEY_R)) {
        InitGame(game);
        return;
    }

    Vector2 mousePos = GetMousePosition();

    // Scratch: place cue ball
    if (game->state == GAME_SCRATCH) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (mousePos.x > RAIL_WIDTH + BALL_RADIUS &&
                mousePos.x < TABLE_WIDTH  - RAIL_WIDTH - BALL_RADIUS &&
                mousePos.y > RAIL_WIDTH + BALL_RADIUS &&
                mousePos.y < TABLE_HEIGHT - RAIL_WIDTH - BALL_RADIUS) {
                game->cueBallPos = mousePos;
                game->balls[0].position = game->cueBallPos;
                game->balls[0].pocketed = false;
                game->balls[0].velocity = (Vector2){0, 0};
                game->state = GAME_PLAYING;
                sprintf(game->statusMessage, "Cue placed. %s's turn", game->players[game->currentPlayer].name);
            } else {
                strcpy(game->statusMessage, "Invalid position! Place inside rails");
            }
        }
        return;
    }

    if (game->ballsMoving) return;

    Vector2 cueBallPos = game->balls[0].pocketed ? game->cueBallPos : game->balls[0].position;

    // Start drag
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (Distance(mousePos, cueBallPos) <= BALL_RADIUS * 1.6f) {
            game->aiming = true;
            game->dragStart = mousePos;
            game->stickPullPixels = 0.0f;
            game->power = 0.0f;
        }
    }

    // Dragging back
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && game->aiming) {
        game->stickPullPixels = Distance(mousePos, cueBallPos);
        game->power = game->stickPullPixels / MAX_POWER_PIXELS;
    }

    // Release — shoot
    if (game->aiming && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        game->aiming = false;

        Vector2 dir = { mousePos.x - cueBallPos.x, mousePos.y - cueBallPos.y };
        float len = sqrtf(dir.x*dir.x + dir.y*dir.y);
        if (len < 0.001f) {
            game->stickPullPixels = 0.0f;
            game->power = 0.0f;
            return;
        }
        dir.x /= len;
        dir.y /= len;

        float shotSpeed = (game->stickPullPixels / MAX_POWER_PIXELS) * MAX_SHOT_SPEED;
        if (shotSpeed > MAX_SHOT_SPEED) shotSpeed = MAX_SHOT_SPEED;

        game->balls[0].velocity.x = dir.x * shotSpeed;
        game->balls[0].velocity.y = dir.y * shotSpeed;

        game->state = GAME_PLAYING;
        game->firstShot = false;
        game->stickRecoil = true;
        game->recoilTimer = STICK_RECOIL_TIME;
        game->power = 0.0f;
    }
}

int playerIndexForType(Game *game, BallType btype) {
    if (btype == BALL_SOLID) {
        if (game->players[0].type == PLAYER_SOLIDS) return 0;
        if (game->players[1].type == PLAYER_SOLIDS) return 1;
    } else if (btype == BALL_STRIPE) {
        if (game->players[0].type == PLAYER_STRIPES) return 0;
        if (game->players[1].type == PLAYER_STRIPES) return 1;
    }
    return -1;
}

void CheckPockets(Game *game) {
    Vector2 pockets[6];
    GetPocketPositions(pockets);

    bool cueBallPocketed = false;
    bool anyPocketed = false;

    for (int i = 0; i < MAX_BALLS; i++) {
        if (game->balls[i].pocketed) continue;

        for (int p = 0; p < 6; p++) {
            if (Distance(game->balls[i].position, pockets[p]) < POCKET_RADIUS) {
                game->balls[i].pocketed = true;
                game->balls[i].velocity = (Vector2){0, 0};
                anyPocketed = true;

                if (i == 0) {
                    cueBallPocketed = true;
                    game->cueBallPos = (Vector2){ TABLE_WIDTH * 0.25f, TABLE_HEIGHT * 0.5f };
                } else {
                    // Assign ball types on first pocket
                    if (!game->assignedTypes) {
                        if (game->balls[i].type == BALL_SOLID) {
                            game->players[game->currentPlayer].type     = PLAYER_SOLIDS;
                            game->players[1 - game->currentPlayer].type = PLAYER_STRIPES;
                            game->assignedTypes = true;
                            sprintf(game->statusMessage, "%s = Solids, %s = Stripes",
                                    game->players[game->currentPlayer].name,
                                    game->players[1 - game->currentPlayer].name);
                        } else if (game->balls[i].type == BALL_STRIPE) {
                            game->players[game->currentPlayer].type     = PLAYER_STRIPES;
                            game->players[1 - game->currentPlayer].type = PLAYER_SOLIDS;
                            game->assignedTypes = true;
                            sprintf(game->statusMessage, "%s = Stripes, %s = Solids",
                                    game->players[game->currentPlayer].name,
                                    game->players[1 - game->currentPlayer].name);
                        }
                    }

                    // 8-ball pocketed
                    if (game->balls[i].type == BALL_EIGHT) {
                        int myIdx = game->currentPlayer;
                        if (game->players[myIdx].ballsRemaining == 0) {
                            game->state = GAME_WON;
                        } else {
                            game->state = GAME_LOST;
                        }
                        return;
                    } else {
                        int ownerIdx = playerIndexForType(game, game->balls[i].type);
                        if (ownerIdx >= 0 && game->players[ownerIdx].ballsRemaining > 0) {
                            game->players[ownerIdx].ballsRemaining--;
                        }
                    }
                }
                break;
            }
        }
    }

    if (cueBallPocketed) ApplyScratch(game);
    if (anyPocketed && !cueBallPocketed) {
        sprintf(game->statusMessage, "%s pocketed a ball!", game->players[game->currentPlayer].name);
    }
}

void ApplyScratch(Game *game) {
    game->state = GAME_SCRATCH;
    strcpy(game->statusMessage, "Scratch! Place cue ball");
    game->currentPlayer = 1 - game->currentPlayer;
}

void CheckWinCondition(Game *game) {
    int idx = game->currentPlayer;
    if (game->players[idx].ballsRemaining == 0) {
        strcpy(game->statusMessage, "Shoot the 8-ball!");
    }
}

void NextTurn(Game *game) {
    game->currentPlayer = 1 - game->currentPlayer;
    sprintf(game->statusMessage, "%s's turn", game->players[game->currentPlayer].name);
}
