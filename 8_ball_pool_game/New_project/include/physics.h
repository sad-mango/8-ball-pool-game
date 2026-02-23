#ifndef PHYSICS_H
#define PHYSICS_H

#include "common.h"

void UpdatePhysics(Game *game);
void CheckCollisions(Game *game);
void ResolveElasticCollision(Ball *a, Ball *b);
void CheckPockets(Game *game);

#endif // PHYSICS_H
