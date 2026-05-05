#ifndef BALL_H
#define BALL_H

#include <raylib.h>
#include <cmath>
#include "../types.h"
#include "../config.h"

class Ball {
public:
    Vector2  position;
    Vector2  velocity;
    Color    color;
    BallType type;
    int      number;
    bool     pocketed;
    bool     isStriped;

    Ball()
        : position{0,0}, velocity{0,0}, color{255,255,255,255},
          type(BallType::SOLID), number(0),
          pocketed(false), isStriped(false) {}

    virtual ~Ball() = default;

    virtual void Draw() const;
    virtual bool BelongsToPlayer() const { return true; }

    bool IsMoving() const {
        return (fabsf(velocity.x) > MIN_VELOCITY ||
                fabsf(velocity.y) > MIN_VELOCITY);
    }

    void Stop() {
        velocity = {0, 0};
        pocketed = true;
    }
};

#endif // BALL_H
