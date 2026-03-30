#ifndef EIGHTBALL_H
#define EIGHTBALL_H

#include "Ball.h"
#include <cstdio>

class EightBall : public Ball {
public:
    EightBall() {
        number    = 8;
        color     = BLACK;
        type      = BallType::EIGHT;
        isStriped = false;
    }

    void Draw() const override {
        DrawCircleV(position, BALL_RADIUS, BLACK);
        char numStr[4];
        std::snprintf(numStr, sizeof(numStr), "%d", number);
        Vector2 tp = {
            position.x - MeasureText(numStr, 12) / 2.0f,
            position.y - 6.0f
        };
        DrawText(numStr, (int)tp.x, (int)tp.y, 12, WHITE);
    }

    bool BelongsToPlayer() const override { return false; }
};

#endif // EIGHTBALL_H
