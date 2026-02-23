#include "utils.h"

float Distance(Vector2 a, Vector2 b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrtf(dx*dx + dy*dy);
}

void ClampBallSpeed(Ball *b, float maxSpeed) {
    float sx = b->velocity.x;
    float sy = b->velocity.y;
    float mag = sqrtf(sx*sx + sy*sy);
    if (mag > maxSpeed) {
        b->velocity.x = (b->velocity.x / mag) * maxSpeed;
        b->velocity.y = (b->velocity.y / mag) * maxSpeed;
    }
}

bool AreBallsMoving(Game *game) {
    for (int i = 0; i < MAX_BALLS; i++) {
        if (game->balls[i].pocketed) continue;
        if (fabs(game->balls[i].velocity.x) > MIN_VELOCITY ||
            fabs(game->balls[i].velocity.y) > MIN_VELOCITY)
            return true;
    }
    return false;
}
