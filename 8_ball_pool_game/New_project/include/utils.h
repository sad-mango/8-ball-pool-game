#ifndef UTILS_H
#define UTILS_H

#include "common.h"

float Distance(Vector2 a, Vector2 b);
void ClampBallSpeed(Ball *b, float maxSpeed);
bool AreBallsMoving(Game *game);

#endif // UTILS_H
