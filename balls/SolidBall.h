#ifndef SOLIDBALL_H
#define SOLIDBALL_H

#include "Ball.h"
#include <cstdio>

class SolidBall : public Ball {
public:
    SolidBall(int num, Color col) {
        number    = num;
        color     = col;
        type      = BallType::SOLID;
        isStriped = false;
    }

    void Draw() const override {
        DrawCircleV(position, BALL_RADIUS, color);
        char numStr[4];
        std::snprintf(numStr, sizeof(numStr), "%d", number);
        Vector2 tp = {
            position.x - MeasureText(numStr, 12) / 2.0f,
            position.y - 6.0f
        };
        DrawText(numStr, (int)tp.x, (int)tp.y, 12, WHITE);
    }
};

#endif // SOLIDBALL_H
