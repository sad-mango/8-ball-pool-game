#ifndef CUEBALL_H
#define CUEBALL_H

#include "Ball.h"

class CueBall : public Ball {
public:
    CueBall() {
        color     = WHITE;
        type      = BallType::CUE;
        number    = 0;
        isStriped = false;
    }

    void Draw() const override {
        DrawCircleV(position, BALL_RADIUS, color);
        DrawCircleV(position, 4, LIGHTGRAY);
    }

    bool BelongsToPlayer() const override { return false; }
};

#endif // CUEBALL_H
