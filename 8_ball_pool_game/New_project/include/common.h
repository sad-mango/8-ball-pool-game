#ifndef COMMON_H
#define COMMON_H

#include <raylib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "config.h"

// --- Enums ---

typedef enum {
    BALL_CUE,
    BALL_SOLID,
    BALL_STRIPE,
    BALL_EIGHT
} BallType;

typedef enum {
    GAME_START,
    GAME_PLAYING,
    GAME_SCRATCH,
    GAME_WON,
    GAME_LOST
} GameState;

typedef enum {
    PLAYER_NONE,
    PLAYER_SOLIDS,
    PLAYER_STRIPES
} PlayerType;

// --- Structs ---

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Color color;
    BallType type;
    int number;
    bool pocketed;
    bool isStriped;
} Ball;

typedef struct {
    PlayerType type;
    int ballsRemaining;
    char name[20];
} Player;

typedef struct {
    Ball balls[MAX_BALLS];
    Player players[2];
    int currentPlayer;
    GameState state;
    Vector2 cueBallPos;
    float power;
    bool aiming;
    bool ballsMoving;
    bool firstShot;
    bool assignedTypes;
    char statusMessage[100];

    Vector2 dragStart;
    float stickPullPixels;
    float stickLength;
    bool stickRecoil;
    float recoilTimer;
} Game;

// Shared pocket positions (used by graphics and game logic)
static inline void GetPocketPositions(Vector2 pockets[6]) {
    pockets[0] = (Vector2){ RAIL_WIDTH,              RAIL_WIDTH };
    pockets[1] = (Vector2){ TABLE_WIDTH * 0.5f,      RAIL_WIDTH };
    pockets[2] = (Vector2){ TABLE_WIDTH - RAIL_WIDTH, RAIL_WIDTH };
    pockets[3] = (Vector2){ RAIL_WIDTH,              TABLE_HEIGHT - RAIL_WIDTH };
    pockets[4] = (Vector2){ TABLE_WIDTH * 0.5f,      TABLE_HEIGHT - RAIL_WIDTH };
    pockets[5] = (Vector2){ TABLE_WIDTH - RAIL_WIDTH, TABLE_HEIGHT - RAIL_WIDTH };
}

#endif // COMMON_H
