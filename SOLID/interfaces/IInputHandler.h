#ifndef IINPUTHANDLER_H
#define IINPUTHANDLER_H

class Game;

class IInputHandler {
public:
    virtual ~IInputHandler() = default;
    virtual void Handle(Game* game) = 0;
};

#endif // IINPUTHANDLER_H
