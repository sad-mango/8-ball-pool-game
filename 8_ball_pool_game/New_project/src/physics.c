#include "physics.h"
#include "utils.h"

void ResolveElasticCollision(Ball *a, Ball *b) {
    float dx = b->position.x - a->position.x;
    float dy = b->position.y - a->position.y;
    float dist = sqrtf(dx*dx + dy*dy);
    if (dist <= 0.0001f) return;

    float nx = dx / dist;
    float ny = dy / dist;
    float tx = -ny;
    float ty =  nx;

    float va_n = a->velocity.x * nx + a->velocity.y * ny;
    float va_t = a->velocity.x * tx + a->velocity.y * ty;
    float vb_n = b->velocity.x * nx + b->velocity.y * ny;
    float vb_t = b->velocity.x * tx + b->velocity.y * ty;

    // Equal-mass elastic: swap normal components
    float va_n_after = vb_n;
    float vb_n_after = va_n;

    a->velocity.x = va_n_after * nx + va_t * tx;
    a->velocity.y = va_n_after * ny + va_t * ty;
    b->velocity.x = vb_n_after * nx + vb_t * tx;
    b->velocity.y = vb_n_after * ny + vb_t * ty;
}

void CheckCollisions(Game *game) {
    for (int i = 0; i < MAX_BALLS; i++) {
        if (game->balls[i].pocketed) continue;
        for (int j = i + 1; j < MAX_BALLS; j++) {
            if (game->balls[j].pocketed) continue;

            float dist = Distance(game->balls[i].position, game->balls[j].position);
            float minDist = BALL_RADIUS * 2.0f;

            if (dist < minDist && dist > 0.0001f) {
                // Separate overlapping balls
                float overlap = 0.5f * (minDist - dist + 0.001f);
                Vector2 normal = {
                    (game->balls[j].position.x - game->balls[i].position.x) / dist,
                    (game->balls[j].position.y - game->balls[i].position.y) / dist
                };
                game->balls[i].position.x -= normal.x * overlap;
                game->balls[i].position.y -= normal.y * overlap;
                game->balls[j].position.x += normal.x * overlap;
                game->balls[j].position.y += normal.y * overlap;

                ResolveElasticCollision(&game->balls[i], &game->balls[j]);

                ClampBallSpeed(&game->balls[i], MAX_BALL_SPEED);
                ClampBallSpeed(&game->balls[j], MAX_BALL_SPEED);
            }
        }
    }
}

void UpdatePhysics(Game *game) {
    for (int i = 0; i < MAX_BALLS; i++) {
        if (game->balls[i].pocketed) continue;

        // Move
        game->balls[i].position.x += game->balls[i].velocity.x;
        game->balls[i].position.y += game->balls[i].velocity.y;

        // Friction
        game->balls[i].velocity.x *= FRICTION;
        game->balls[i].velocity.y *= FRICTION;

        // Stop very slow balls
        if (fabs(game->balls[i].velocity.x) < MIN_VELOCITY) game->balls[i].velocity.x = 0;
        if (fabs(game->balls[i].velocity.y) < MIN_VELOCITY) game->balls[i].velocity.y = 0;

        // Rail bounces
        if (game->balls[i].position.x - BALL_RADIUS < RAIL_WIDTH) {
            game->balls[i].position.x = RAIL_WIDTH + BALL_RADIUS;
            game->balls[i].velocity.x = -game->balls[i].velocity.x * 0.86f;
        }
        if (game->balls[i].position.x + BALL_RADIUS > TABLE_WIDTH - RAIL_WIDTH) {
            game->balls[i].position.x = TABLE_WIDTH - RAIL_WIDTH - BALL_RADIUS;
            game->balls[i].velocity.x = -game->balls[i].velocity.x * 0.86f;
        }
        if (game->balls[i].position.y - BALL_RADIUS < RAIL_WIDTH) {
            game->balls[i].position.y = RAIL_WIDTH + BALL_RADIUS;
            game->balls[i].velocity.y = -game->balls[i].velocity.y * 0.86f;
        }
        if (game->balls[i].position.y + BALL_RADIUS > TABLE_HEIGHT - RAIL_WIDTH) {
            game->balls[i].position.y = TABLE_HEIGHT - RAIL_WIDTH - BALL_RADIUS;
            game->balls[i].velocity.y = -game->balls[i].velocity.y * 0.86f;
        }

        ClampBallSpeed(&game->balls[i], MAX_BALL_SPEED);
    }

    CheckCollisions(game);
    CheckPockets(game);
}
