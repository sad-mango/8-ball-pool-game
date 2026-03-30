#ifndef RENDERER_H
#define RENDERER_H

#include "../interfaces/IRenderer.h"
#include "../config.h"

class Renderer : public IRenderer {
public:
    void Draw(Game* game) override;

private:
    void DrawTable();
    void DrawPockets();
    void DrawBalls(Game* game);
    void DrawCueStick(Game* game);
    void DrawPowerBar(Game* game);
    void DrawHUD(Game* game);
    void DrawOverlays(Game* game);
};

#endif // RENDERER_H
