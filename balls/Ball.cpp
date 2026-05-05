#include "Ball.h"
#include <cstdio>

void Ball::Draw() const {
    DrawCircleV(position, BALL_RADIUS, color);
    char numStr[4];
    std::snprintf(numStr, sizeof(numStr), "%d", number);
    Vector2 tp = {
        position.x - MeasureText(numStr, 12) / 2.0f,
        position.y - 6.0f
    };
    DrawText(numStr, (int)tp.x, (int)tp.y, 12, WHITE);
}
