#ifndef GAME_H
#define GAME_H

#include <string>
#include <raylib.h>
#include "../types.h"
#include "../config.h"
#include "../managers/BallManager.h"
#include "../managers/TurnManager.h"
#include "../interfaces/IPhysics.h"
#include "../interfaces/IRenderer.h"
#include "../interfaces/IInputHandler.h"

class Game {
public:
    IPhysics*      physics;
    IRenderer*     renderer;
    IInputHandler* input;

    BallManager  ballManager;
    TurnManager  turnManager;

    GameState   state;
    std::string statusMessage;
    Vector2     cueBallPos;

    float   power;
    bool    aiming;
    bool    ballsMoving;
    bool    firstShot;
    Vector2 dragStart;
    float   stickPullPixels;
    float   stickLength;
    bool    stickRecoil;
    float   recoilTimer;

    Game(IPhysics* phys, IRenderer* rend, IInputHandler* inp)
        : physics(phys), renderer(rend), input(inp),
          state(GameState::START),
          cueBallPos{TABLE_WIDTH * 0.25f, TABLE_HEIGHT * 0.5f},
          power(0.0f), aiming(false), ballsMoving(false), firstShot(true),
          dragStart{0,0}, stickPullPixels(0.0f), stickLength(STICK_LENGTH),
          stickRecoil(false), recoilTimer(0.0f)
    {
        statusMessage = "Break shot: click on cue, drag back, release to shoot";
        ballManager.Reset();
    }

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

    void Update() {
        input->Handle(this);

        if (stickRecoil) {
            recoilTimer -= 1.0f / TARGET_FPS;
            if (recoilTimer <= 0.0f) {
                stickRecoil     = false;
                stickPullPixels = 0.0f;
            } else {
                stickPullPixels *= 0.92f;
                power = stickPullPixels / MAX_POWER_PIXELS;
                if (power < 0) power = 0;
            }
        }

        if (state == GameState::PLAYING || state == GameState::SCRATCH) {
            physics->Update(this);

            if (!ballsMoving && ballManager.AnyMoving()) ballsMoving = true;

            if (ballsMoving && !ballManager.AnyMoving()) {
                ballsMoving = false;
                if (state == GameState::PLAYING) {
                    CheckWinCondition();
                    if (state != GameState::WON && state != GameState::LOST)
                        NextTurn();
                }
            }
        }
    }

    void Draw() { renderer->Draw(this); }

    void NextTurn() {
        turnManager.NextTurn();
        statusMessage = turnManager.Current().name + "'s turn";
    }

    void ApplyScratch() {
        state = GameState::SCRATCH;
        statusMessage = "Scratch! Place cue ball";
        turnManager.SwitchOnScratch();
    }

    void CheckWinCondition() {
        if (turnManager.Current().ballsRemaining == 0)
            statusMessage = "Shoot the 8-ball!";
    }
};

#endif // GAME_H
