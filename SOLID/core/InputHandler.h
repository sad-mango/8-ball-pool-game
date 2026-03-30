#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include "../interfaces/IInputHandler.h"
#include "../config.h"
#include <raylib.h>
#include <cmath>

class InputHandler : public IInputHandler {
public:
    void Handle(Game* game) override;

private:
    void HandleScratchPlacement(Game* game);
    void HandleAiming(Game* game);
    void HandleShoot(Game* game);

    float Distance(Vector2 a, Vector2 b) const {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return sqrtf(dx*dx + dy*dy);
    }
};

#endif // INPUTHANDLER_H
