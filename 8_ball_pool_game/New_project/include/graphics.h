#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "common.h"

void DrawGame(Game *game);
void DrawTable(void);
void DrawBalls(Game *game);
void DrawCueStick(Game *game);
void DrawPowerBar(Game *game);
void DrawHUD(Game *game);
void DrawOverlays(Game *game);

#endif // GRAPHICS_H
