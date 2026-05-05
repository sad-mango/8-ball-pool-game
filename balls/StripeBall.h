#ifndef STRIPEBALL_H
#define STRIPEBALL_H

#include "Ball.h"
#include <cstdio>

class StripeBall : public Ball {
public:
    StripeBall(int num, Color col) {
        number    = num;
        color     = col;
        type      = BallType::STRIPE;
        isStriped = true;
    }

    void Draw() const override {
        DrawCircleV(position, BALL_RADIUS, color);
        DrawRectangleV(
            { position.x - BALL_RADIUS * 0.9f, position.y - BALL_RADIUS * 0.28f },
            { BALL_RADIUS * 1.8f, BALL_RADIUS * 0.56f },
            WHITE);
        DrawCircleV(position, BALL_RADIUS - 1, color);
        char numStr[4];
        std::snprintf(numStr, sizeof(numStr), "%d", number);
        Vector2 tp = {
            position.x - MeasureText(numStr, 12) / 2.0f,
            position.y - 6.0f
        };
        DrawText(numStr, (int)tp.x, (int)tp.y, 12, WHITE);
    }
};

#endif // STRIPEBALL_H
