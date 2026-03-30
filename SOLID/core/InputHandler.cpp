#include "InputHandler.h"
#include "Game.h"
#include <cmath>

void InputHandler::Handle(Game* game) {
    if (IsKeyPressed(KEY_R)) {
        game->Init();
        return;
    }

    if (game->state == GameState::SCRATCH) {
        HandleScratchPlacement(game);
        return;
    }

    if (game->ballsMoving) return;

    HandleAiming(game);
    HandleShoot(game);
}

void InputHandler::HandleScratchPlacement(Game* game) {
    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return;

    Vector2 mousePos = GetMousePosition();

    bool insideRails = (mousePos.x > RAIL_WIDTH + BALL_RADIUS &&
                        mousePos.x < TABLE_WIDTH  - RAIL_WIDTH - BALL_RADIUS &&
                        mousePos.y > RAIL_WIDTH + BALL_RADIUS &&
                        mousePos.y < TABLE_HEIGHT - RAIL_WIDTH - BALL_RADIUS);

    if (insideRails) {
        Ball* cue        = game->ballManager.GetCueBall();
        game->cueBallPos = mousePos;
        cue->position    = mousePos;
        cue->pocketed    = false;
        cue->velocity    = {0, 0};
        game->state      = GameState::PLAYING;
        game->statusMessage = "Cue placed. " + game->turnManager.Current().name + "'s turn";
    } else {
        game->statusMessage = "Invalid position! Place inside rails";
    }
}

void InputHandler::HandleAiming(Game* game) {
    Vector2 mousePos   = GetMousePosition();
    Ball*   cue        = game->ballManager.GetCueBall();
    Vector2 cueBallPos = (cue && !cue->pocketed) ? cue->position : game->cueBallPos;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (Distance(mousePos, cueBallPos) <= BALL_RADIUS * 1.6f) {
            game->aiming          = true;
            game->dragStart       = mousePos;
            game->stickPullPixels = 0.0f;
            game->power           = 0.0f;
        }
    }

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && game->aiming) {
        game->stickPullPixels = Distance(mousePos, cueBallPos);
        game->power           = game->stickPullPixels / MAX_POWER_PIXELS;
    }
}

void InputHandler::HandleShoot(Game* game) {
    if (!game->aiming) return;
    if (!IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) return;

    game->aiming = false;

    Ball*   cue        = game->ballManager.GetCueBall();
    Vector2 cueBallPos = (cue && !cue->pocketed) ? cue->position : game->cueBallPos;
    Vector2 mousePos   = GetMousePosition();

    Vector2 dir = { mousePos.x - cueBallPos.x, mousePos.y - cueBallPos.y };
    float len = sqrtf(dir.x*dir.x + dir.y*dir.y);

    if (len < 0.001f) {
        game->stickPullPixels = 0.0f;
        game->power           = 0.0f;
        return;
    }

    dir.x /= len;
    dir.y /= len;

    float shotSpeed = (game->stickPullPixels / MAX_POWER_PIXELS) * MAX_SHOT_SPEED;
    if (shotSpeed > MAX_SHOT_SPEED) shotSpeed = MAX_SHOT_SPEED;

    if (cue) {
        cue->velocity.x = dir.x * shotSpeed;
        cue->velocity.y = dir.y * shotSpeed;
    }

    game->state         = GameState::PLAYING;
    game->firstShot     = false;
    game->stickRecoil   = true;
    game->recoilTimer   = STICK_RECOIL_TIME;
    game->power         = 0.0f;
}
