#ifndef IRENDERER_H
#define IRENDERER_H

class Game;

class IRenderer {
public:
    virtual ~IRenderer() = default;
    virtual void Draw(Game* game) = 0;
};

#endif // IRENDERER_H
