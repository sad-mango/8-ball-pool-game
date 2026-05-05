#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

// =============================================================
//  GameController  -  CONTROLLER layer of MVC
//
//  Reads keyboard/mouse input via raylib.
//  Calls physics to advance simulation.
//  Writes ALL resulting state changes into GameModel.
//  NEVER draws anything - that is GameView's responsibility.
//
//  All logic migrated from:
//    core/InputHandler.cpp  (HandleInput family)
//    core/Game.h            (Update / physics loop)
//    core/PhysicsEngine.cpp (simulation)
// =============================================================

#include <cmath>
#include <string>
#include <raylib.h>
#include "GameModel.h"
#include "../config.h"
#include "../balls/Ball.h"

class GameController {
public:
    // Called once per frame
    void Update(GameModel& model) {
        HandleInput(model);
        UpdatePhysics(model);
    }

private:
    // ===================== INPUT ==============================

    void HandleInput(GameModel& model) {
        // R always restarts
        if (IsKeyPressed(KEY_R)) {
            model.Init();
            return;
        }

        // After scratch: place cue ball mode
        if (model.state == GameState::SCRATCH) {
            HandleScratchPlacement(model);
            return;
        }

        // No input while balls rolling
        if (model.ballsMoving) return;

        HandleAiming(model);
        HandleShoot(model);
    }

    // Click inside rails to place cue ball after a scratch
    void HandleScratchPlacement(GameModel& model) {
        if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return;

        Vector2 mousePos = GetMousePosition();

        bool insideRails = (mousePos.x > RAIL_WIDTH + BALL_RADIUS &&
                            mousePos.x < TABLE_WIDTH  - RAIL_WIDTH - BALL_RADIUS &&
                            mousePos.y > RAIL_WIDTH + BALL_RADIUS &&
                            mousePos.y < TABLE_HEIGHT - RAIL_WIDTH - BALL_RADIUS);

        if (insideRails) {
            Ball* cue        = model.ballManager.GetCueBall();
            model.cueBallPos = mousePos;
            cue->position    = mousePos;
            cue->pocketed    = false;
            cue->velocity    = { 0, 0 };
            model.state      = GameState::PLAYING;
            model.statusMessage = "Cue placed. " +
                                  model.turnManager.Current().name + "'s turn";
        } else {
            model.statusMessage = "Invalid position! Place inside rails";
        }
    }

    // Drag back from cue ball to set direction + power
    void HandleAiming(GameModel& model) {
        Vector2 mousePos   = GetMousePosition();
        Ball*   cue        = model.ballManager.GetCueBall();
        Vector2 cueBallPos = (cue && !cue->pocketed) ? cue->position : model.cueBallPos;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (Distance(mousePos, cueBallPos) <= BALL_RADIUS * 1.6f) {
                model.aiming          = true;
                model.dragStart       = mousePos;
                model.stickPullPixels = 0.0f;
                model.power           = 0.0f;
            }
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && model.aiming) {
            model.stickPullPixels = Distance(mousePos, cueBallPos);
            model.power           = model.stickPullPixels / MAX_POWER_PIXELS;
        }
    }

    // Release mouse to fire cue ball
    void HandleShoot(GameModel& model) {
        if (!model.aiming) return;
        if (!IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) return;

        model.aiming = false;

        Ball*   cue        = model.ballManager.GetCueBall();
        Vector2 cueBallPos = (cue && !cue->pocketed) ? cue->position : model.cueBallPos;
        Vector2 mousePos   = GetMousePosition();

        Vector2 dir = { mousePos.x - cueBallPos.x, mousePos.y - cueBallPos.y };
        float len   = sqrtf(dir.x * dir.x + dir.y * dir.y);

        if (len < 0.001f) {
            model.stickPullPixels = 0.0f;
            model.power           = 0.0f;
            return;
        }

        dir.x /= len;
        dir.y /= len;

        float shotSpeed = (model.stickPullPixels / MAX_POWER_PIXELS) * MAX_SHOT_SPEED;
        if (shotSpeed > MAX_SHOT_SPEED) shotSpeed = MAX_SHOT_SPEED;

        if (cue) {
            cue->velocity.x = dir.x * shotSpeed;
            cue->velocity.y = dir.y * shotSpeed;
        }

        model.state       = GameState::PLAYING;
        model.firstShot   = false;
        model.stickRecoil = true;
        model.recoilTimer = STICK_RECOIL_TIME;
        model.power       = 0.0f;
    }

    // ===================== PHYSICS ============================

    void UpdatePhysics(GameModel& model) {
        // Animate cue-stick recoil animation
        if (model.stickRecoil) {
            model.recoilTimer -= 1.0f / TARGET_FPS;
            if (model.recoilTimer <= 0.0f) {
                model.stickRecoil     = false;
                model.stickPullPixels = 0.0f;
            } else {
                model.stickPullPixels *= 0.92f;
                model.power = model.stickPullPixels / MAX_POWER_PIXELS;
                if (model.power < 0) model.power = 0;
            }
        }

        if (model.state == GameState::PLAYING || model.state == GameState::SCRATCH) {
            // Move balls and check collisions/pockets
            MoveBalls(model);
            CheckCollisions(model);
            CheckPockets(model);

            // Detect transition: balls just stopped
            if (!model.ballsMoving && model.ballManager.AnyMoving())
                model.ballsMoving = true;

            if (model.ballsMoving && !model.ballManager.AnyMoving()) {
                model.ballsMoving = false;
                if (model.state == GameState::PLAYING) {
                    model.CheckWinCondition();
                    if (model.state != GameState::WON && model.state != GameState::LOST)
                        model.NextTurn();
                }
            }
        }
    }

    // Move each ball and bounce off rails
    void MoveBalls(GameModel& model) {
        for (int i = 0; i < MAX_BALLS; i++) {
            Ball* b = model.ballManager.balls[i];
            if (!b || b->pocketed) continue;

            b->position.x += b->velocity.x;
            b->position.y += b->velocity.y;

            b->velocity.x *= FRICTION;
            b->velocity.y *= FRICTION;

            if (fabsf(b->velocity.x) < MIN_VELOCITY) b->velocity.x = 0;
            if (fabsf(b->velocity.y) < MIN_VELOCITY) b->velocity.y = 0;

            // Rail bouncing
            if (b->position.x - BALL_RADIUS < RAIL_WIDTH) {
                b->position.x  = RAIL_WIDTH + BALL_RADIUS;
                b->velocity.x *= -0.86f;
            }
            if (b->position.x + BALL_RADIUS > TABLE_WIDTH - RAIL_WIDTH) {
                b->position.x  = TABLE_WIDTH - RAIL_WIDTH - BALL_RADIUS;
                b->velocity.x *= -0.86f;
            }
            if (b->position.y - BALL_RADIUS < RAIL_WIDTH) {
                b->position.y  = RAIL_WIDTH + BALL_RADIUS;
                b->velocity.y *= -0.86f;
            }
            if (b->position.y + BALL_RADIUS > TABLE_HEIGHT - RAIL_WIDTH) {
                b->position.y  = TABLE_HEIGHT - RAIL_WIDTH - BALL_RADIUS;
                b->velocity.y *= -0.86f;
            }

            ClampSpeed(b, MAX_BALL_SPEED);
        }
    }

    // Ball-ball collision detection and resolution
    void CheckCollisions(GameModel& model) {
        for (int i = 0; i < MAX_BALLS; i++) {
            Ball* a = model.ballManager.balls[i];
            if (!a || a->pocketed) continue;

            for (int j = i + 1; j < MAX_BALLS; j++) {
                Ball* b = model.ballManager.balls[j];
                if (!b || b->pocketed) continue;

                float dist    = Distance(a->position, b->position);
                float minDist = BALL_RADIUS * 2.0f;

                if (dist < minDist && dist > 0.0001f) {
                    // Push apart
                    float overlap = 0.5f * (minDist - dist + 0.001f);
                    Vector2 normal = {
                        (b->position.x - a->position.x) / dist,
                        (b->position.y - a->position.y) / dist
                    };
                    a->position.x -= normal.x * overlap;
                    a->position.y -= normal.y * overlap;
                    b->position.x += normal.x * overlap;
                    b->position.y += normal.y * overlap;

                    // Elastic velocity swap
                    ResolveElasticCollision(a, b);
                    ClampSpeed(a, MAX_BALL_SPEED);
                    ClampSpeed(b, MAX_BALL_SPEED);
                }
            }
        }
    }

    // Equal-mass elastic collision
    void ResolveElasticCollision(Ball* a, Ball* b) {
        float dx   = b->position.x - a->position.x;
        float dy   = b->position.y - a->position.y;
        float dist = sqrtf(dx * dx + dy * dy);
        if (dist <= 0.0001f) return;

        float nx = dx / dist, ny = dy / dist;
        float tx = -ny,       ty =  nx;

        float va_n = a->velocity.x * nx + a->velocity.y * ny;
        float va_t = a->velocity.x * tx + a->velocity.y * ty;
        float vb_n = b->velocity.x * nx + b->velocity.y * ny;
        float vb_t = b->velocity.x * tx + b->velocity.y * ty;

        // Swap normal components (equal mass)
        a->velocity.x = vb_n * nx + va_t * tx;
        a->velocity.y = vb_n * ny + va_t * ty;
        b->velocity.x = va_n * nx + vb_t * tx;
        b->velocity.y = va_n * ny + vb_t * ty;
    }

    // Check if balls fall into pockets
    void CheckPockets(GameModel& model) {
        Vector2 pockets[6] = {
            { RAIL_WIDTH,                RAIL_WIDTH },
            { TABLE_WIDTH * 0.5f,        RAIL_WIDTH },
            { TABLE_WIDTH - RAIL_WIDTH,  RAIL_WIDTH },
            { RAIL_WIDTH,                TABLE_HEIGHT - RAIL_WIDTH },
            { TABLE_WIDTH * 0.5f,        TABLE_HEIGHT - RAIL_WIDTH },
            { TABLE_WIDTH - RAIL_WIDTH,  TABLE_HEIGHT - RAIL_WIDTH }
        };

        bool cueBallPocketed = false;
        bool anyPocketed     = false;

        for (int i = 0; i < MAX_BALLS; i++) {
            Ball* ball = model.ballManager.balls[i];
            if (!ball || ball->pocketed) continue;

            for (int p = 0; p < 6; p++) {
                if (Distance(ball->position, pockets[p]) < POCKET_RADIUS) {
                    ball->Stop();
                    anyPocketed = true;

                    if (ball->type == BallType::CUE) {
                        cueBallPocketed  = true;
                        model.cueBallPos = { TABLE_WIDTH * 0.25f, TABLE_HEIGHT * 0.5f };
                    } else if (ball->type == BallType::EIGHT) {
                        // Win if cleared your set, lose otherwise
                        model.state = (model.turnManager.Current().ballsRemaining == 0)
                                      ? GameState::WON
                                      : GameState::LOST;
                        return;
                    } else {
                        std::string msg = model.turnManager.AssignTypes(ball->type);
                        if (!msg.empty()) model.statusMessage = msg;

                        int ownerIdx = model.turnManager.PlayerIndexForType(ball->type);
                        if (ownerIdx >= 0 &&
                            model.turnManager.players[ownerIdx].ballsRemaining > 0)
                            model.turnManager.players[ownerIdx].ballsRemaining--;
                    }
                    break;
                }
            }
        }

        if (cueBallPocketed)
            model.ApplyScratch();
        else if (anyPocketed)
            model.statusMessage = model.turnManager.Current().name + " pocketed a ball!";
    }

    // ===================== UTILITIES ==========================

    float Distance(Vector2 a, Vector2 b) const {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return sqrtf(dx * dx + dy * dy);
    }

    void ClampSpeed(Ball* b, float maxSpeed) const {
        float mag = sqrtf(b->velocity.x * b->velocity.x +
                          b->velocity.y * b->velocity.y);
        if (mag > maxSpeed) {
            b->velocity.x = (b->velocity.x / mag) * maxSpeed;
            b->velocity.y = (b->velocity.y / mag) * maxSpeed;
        }
    }
};

#endif // GAMECONTROLLER_H
