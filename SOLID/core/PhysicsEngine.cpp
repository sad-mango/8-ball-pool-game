#include <cmath>
#include "PhysicsEngine.h"
#include "Game.h"

void PhysicsEngine::Update(Game* game) {
    for (int i = 0; i < MAX_BALLS; i++) {
        Ball* b = game->ballManager.balls[i];
        if (!b || b->pocketed) continue;
        MoveAndBounce(b);
    }
    CheckCollisions(game);
    CheckPockets(game);
}

void PhysicsEngine::MoveAndBounce(Ball* b) {
    b->position.x += b->velocity.x;
    b->position.y += b->velocity.y;

    b->velocity.x *= FRICTION;
    b->velocity.y *= FRICTION;

    if (fabsf(b->velocity.x) < MIN_VELOCITY) b->velocity.x = 0;
    if (fabsf(b->velocity.y) < MIN_VELOCITY) b->velocity.y = 0;

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

void PhysicsEngine::ResolveElasticCollision(Ball* a, Ball* b) {
    float dx   = b->position.x - a->position.x;
    float dy   = b->position.y - a->position.y;
    float dist = sqrtf(dx*dx + dy*dy);
    if (dist <= 0.0001f) return;

    float nx = dx / dist,  ny = dy / dist;
    float tx = -ny,        ty =  nx;

    float va_n = a->velocity.x * nx + a->velocity.y * ny;
    float va_t = a->velocity.x * tx + a->velocity.y * ty;
    float vb_n = b->velocity.x * nx + b->velocity.y * ny;
    float vb_t = b->velocity.x * tx + b->velocity.y * ty;

    // Equal-mass elastic: swap normal components
    a->velocity.x = vb_n * nx + va_t * tx;
    a->velocity.y = vb_n * ny + va_t * ty;
    b->velocity.x = va_n * nx + vb_t * tx;
    b->velocity.y = va_n * ny + vb_t * ty;
}

void PhysicsEngine::CheckCollisions(Game* game) {
    for (int i = 0; i < MAX_BALLS; i++) {
        Ball* a = game->ballManager.balls[i];
        if (!a || a->pocketed) continue;

        for (int j = i + 1; j < MAX_BALLS; j++) {
            Ball* b = game->ballManager.balls[j];
            if (!b || b->pocketed) continue;

            float dist    = Distance(a->position, b->position);
            float minDist = BALL_RADIUS * 2.0f;

            if (dist < minDist && dist > 0.0001f) {
                float overlap = 0.5f * (minDist - dist + 0.001f);
                Vector2 normal = {
                    (b->position.x - a->position.x) / dist,
                    (b->position.y - a->position.y) / dist
                };
                a->position.x -= normal.x * overlap;
                a->position.y -= normal.y * overlap;
                b->position.x += normal.x * overlap;
                b->position.y += normal.y * overlap;

                ResolveElasticCollision(a, b);
                ClampSpeed(a, MAX_BALL_SPEED);
                ClampSpeed(b, MAX_BALL_SPEED);
            }
        }
    }
}

void PhysicsEngine::CheckPockets(Game* game) {
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
        Ball* ball = game->ballManager.balls[i];
        if (!ball || ball->pocketed) continue;

        for (int p = 0; p < 6; p++) {
            if (Distance(ball->position, pockets[p]) < POCKET_RADIUS) {
                ball->Stop();
                anyPocketed = true;

                if (ball->type == BallType::CUE) {
                    cueBallPocketed  = true;
                    game->cueBallPos = { TABLE_WIDTH * 0.25f, TABLE_HEIGHT * 0.5f };
                } else if (ball->type == BallType::EIGHT) {
                    game->state = (game->turnManager.Current().ballsRemaining == 0)
                                  ? GameState::WON
                                  : GameState::LOST;
                    return;
                } else {
                    std::string msg = game->turnManager.AssignTypes(ball->type);
                    if (!msg.empty()) game->statusMessage = msg;

                    int ownerIdx = game->turnManager.PlayerIndexForType(ball->type);
                    if (ownerIdx >= 0 &&
                        game->turnManager.players[ownerIdx].ballsRemaining > 0)
                        game->turnManager.players[ownerIdx].ballsRemaining--;
                }
                break;
            }
        }
    }

    if (cueBallPocketed) game->ApplyScratch();
    else if (anyPocketed)
        game->statusMessage = game->turnManager.Current().name + " pocketed a ball!";
}
