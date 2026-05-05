#ifndef GAME_H
#define GAME_H

#include "common.h"

void InitGame(Game *game);
void ResetBalls(Game *game);
void UpdateGame(Game *game);
void HandleInput(Game *game);
void CheckPockets(Game *game);
void CheckWinCondition(Game *game);
void NextTurn(Game *game);
void ApplyScratch(Game *game);
int  playerIndexForType(Game *game, BallType btype);

#endif // GAME_H
