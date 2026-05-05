#ifndef BALLMANAGER_H
#define BALLMANAGER_H

#include "../balls/Ball.h"
#include "../balls/CueBall.h"
#include "../balls/SolidBall.h"
#include "../balls/StripeBall.h"
#include "../balls/EightBall.h"
#include "../config.h"

class BallManager {
public:
    Ball* balls[MAX_BALLS];

    BallManager() {
        for (int i = 0; i < MAX_BALLS; i++) balls[i] = nullptr;
    }

    ~BallManager() {
        for (int i = 0; i < MAX_BALLS; i++) {
            delete balls[i];
            balls[i] = nullptr;
        }
    }

    // Disable copy to avoid double-delete
    BallManager(const BallManager&) = delete;
    BallManager& operator=(const BallManager&) = delete;

    void Reset() {
        for (int i = 0; i < MAX_BALLS; i++) {
            delete balls[i];
            balls[i] = nullptr;
        }

        Color solidColors[]  = { VIOLET, BLUE, RED, PURPLE, ORANGE, DARKBLUE, MAROON };
        Color stripeColors[] = { VIOLET, BLUE, RED, PURPLE, ORANGE, DARKBLUE, MAROON };

        balls[0] = new CueBall();
        balls[0]->position = { TABLE_WIDTH * 0.25f, TABLE_HEIGHT * 0.5f };

        Vector2 triangleStart = { TABLE_WIDTH * 0.72f, TABLE_HEIGHT * 0.5f };
        int idx = 1;
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col <= row; col++) {
                if (idx >= MAX_BALLS) break;
                float offsetX = row * (BALL_RADIUS * 2 * 0.88f);
                float offsetY = (col * (BALL_RADIUS * 2)) - (row * BALL_RADIUS);
                Vector2 pos   = { triangleStart.x + offsetX, triangleStart.y + offsetY };

                if (idx == 8) {
                    balls[idx] = new EightBall();
                } else if (idx <= 7) {
                    balls[idx] = new SolidBall(idx, solidColors[idx - 1]);
                } else {
                    int sidx = idx - 9;
                    if (sidx < 0) sidx = 0;
                    balls[idx] = new StripeBall(idx, stripeColors[sidx]);
                }

                balls[idx]->position = pos;
                balls[idx]->velocity = {0, 0};
                balls[idx]->number   = idx;
                idx++;
            }
        }
    }

    bool AnyMoving() const {
        for (int i = 0; i < MAX_BALLS; i++) {
            if (balls[i] && !balls[i]->pocketed && balls[i]->IsMoving())
                return true;
        }
        return false;
    }

    Ball* GetCueBall() const { return balls[0]; }
};

#endif // BALLMANAGER_H
