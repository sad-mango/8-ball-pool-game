#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include "../types.h"

struct Player {
    std::string name;
    PlayerType  type;
    int         ballsRemaining;

    Player() : type(PlayerType::NONE), ballsRemaining(7) {}
};

#endif // PLAYER_H
