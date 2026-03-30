#ifndef PHYSICSENGINE_H
#define PHYSICSENGINE_H

#include "../interfaces/IPhysics.h"
#include "../balls/Ball.h"
#include "../config.h"
#include <cmath>

class Game;

class PhysicsEngine : public IPhysics {
public:
    void Update(Game* game) override;

private:
    void MoveAndBounce(Ball* b);
    void ResolveElasticCollision(Ball* a, Ball* b);
    void CheckCollisions(Game* game);
    void CheckPockets(Game* game);

    float Distance(Vector2 a, Vector2 b) const {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return sqrtf(dx*dx + dy*dy);
    }

    void ClampSpeed(Ball* b, float maxSpeed) const {
        float mag = sqrtf(b->velocity.x * b->velocity.x +
                          b->velocity.y * b->velocity.y);
        if (mag > maxSpeed) {
            b->velocity.x = (b->velocity.x / mag) * maxSpeed;
            b->velocity.y = (b->velocity.y / mag) * maxSpeed;
        }
    }
};

#endif // PHYSICSENGINE_H
