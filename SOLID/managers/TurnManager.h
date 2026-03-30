#ifndef TURNMANAGER_H
#define TURNMANAGER_H

#include <string>
#include "Player.h"
#include "../types.h"

class TurnManager {
public:
    Player players[2];
    int    currentPlayer;
    bool   assignedTypes;

    TurnManager() : currentPlayer(0), assignedTypes(false) {
        players[0].name           = "Player 1";
        players[0].ballsRemaining = 7;
        players[1].name           = "Player 2";
        players[1].ballsRemaining = 7;
    }

    void NextTurn()        { currentPlayer = 1 - currentPlayer; }
    void SwitchOnScratch() { currentPlayer = 1 - currentPlayer; }

    std::string AssignTypes(BallType pocketedType) {
        if (assignedTypes) return "";
        if (pocketedType == BallType::SOLID) {
            players[currentPlayer].type     = PlayerType::SOLIDS;
            players[1 - currentPlayer].type = PlayerType::STRIPES;
            assignedTypes = true;
            return players[currentPlayer].name + " = Solids, " +
                   players[1 - currentPlayer].name + " = Stripes";
        } else if (pocketedType == BallType::STRIPE) {
            players[currentPlayer].type     = PlayerType::STRIPES;
            players[1 - currentPlayer].type = PlayerType::SOLIDS;
            assignedTypes = true;
            return players[currentPlayer].name + " = Stripes, " +
                   players[1 - currentPlayer].name + " = Solids";
        }
        return "";
    }

    int PlayerIndexForType(BallType btype) const {
        for (int i = 0; i < 2; i++) {
            if (btype == BallType::SOLID  && players[i].type == PlayerType::SOLIDS)  return i;
            if (btype == BallType::STRIPE && players[i].type == PlayerType::STRIPES) return i;
        }
        return -1;
    }

    Player&       Current()       { return players[currentPlayer]; }
    const Player& Current() const { return players[currentPlayer]; }
    Player&       Other()         { return players[1 - currentPlayer]; }
};

#endif // TURNMANAGER_H
