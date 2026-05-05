#ifndef GAMEMODEL_H
#define GAMEMODEL_H

// =============================================================
//  GameModel  -  MODEL layer of MVC
//
//  Owns ALL game state: balls, players, turn info, aiming.
//  Has NO knowledge of rendering or input.
//  All other classes read from or write to this class.
// =============================================================

#include <string>
#include <raylib.h>
#include "../types.h"
#include "../config.h"
#include "../managers/BallManager.h"
#include "../managers/TurnManager.h"

class GameModel {
public:
    // ---- Game state ------------------------------------------
    GameState   state;
    std::string statusMessage;
    Vector2     cueBallPos;

    // ---- Aiming / cue stick state ----------------------------
    float   power;
    bool    aiming;
    bool    ballsMoving;
    bool    firstShot;
    Vector2 dragStart;
    float   stickPullPixels;
    float   stickLength;
    bool    stickRecoil;
    float   recoilTimer;

    // ---- Domain objects (from managers/) ---------------------
    BallManager ballManager;
    TurnManager turnManager;

    // ---- Constructor -----------------------------------------
    GameModel()
        : state(GameState::START),
          cueBallPos{ TABLE_WIDTH * 0.25f, TABLE_HEIGHT * 0.5f },
          power(0.0f), aiming(false), ballsMoving(false), firstShot(true),
          dragStart{0,0}, stickPullPixels(0.0f), stickLength(STICK_LENGTH),
          stickRecoil(false), recoilTimer(0.0f)
    {
        statusMessage = "Break shot: click on cue, drag back, release to shoot";
        ballManager.Reset();
    }

    // ---- Reset all state to start a new game -----------------
    void Init() {
        turnManager     = TurnManager();
        state           = GameState::START;
        power           = 0.0f;
        aiming          = false;
        ballsMoving     = false;
        firstShot       = true;
        stickPullPixels = 0.0f;
        stickLength     = STICK_LENGTH;
        stickRecoil     = false;
        recoilTimer     = 0.0f;
        statusMessage   = "Break shot: click on cue, drag back, release to shoot";
        cueBallPos      = { TABLE_WIDTH * 0.25f, TABLE_HEIGHT * 0.5f };
        ballManager.Reset();
    }

    // ---- Advance to next player's turn -----------------------
    void NextTurn() {
        turnManager.NextTurn();
        statusMessage = turnManager.Current().name + "'s turn";
    }

    // ---- Handle cue ball scratch -----------------------------
    void ApplyScratch() {
        state         = GameState::SCRATCH;
        statusMessage = "Scratch! Place cue ball";
        turnManager.SwitchOnScratch();
    }

    // ---- Check if current player can go for 8-ball -----------
    void CheckWinCondition() {
        if (turnManager.Current().ballsRemaining == 0)
            statusMessage = "Shoot the 8-ball!";
    }
};

#endif // GAMEMODEL_H
