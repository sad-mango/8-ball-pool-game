#ifndef IPHYSICS_H
#define IPHYSICS_H

class Game;

class IPhysics {
public:
    virtual ~IPhysics() = default;
    virtual void Update(Game* game) = 0;
};

#endif // IPHYSICS_H
