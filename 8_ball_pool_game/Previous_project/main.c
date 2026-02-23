#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_BL 16
#define T_W 800
#define T_H 400
#define BL_R 15
#define PKT_R 28
#define RIL_W 40

// Physics tuning
#define FRIC 0.985f
#define MIN_VEL 0.06f
#define MAX_PWR_PX 160.0f  
#define MAX_SHT_SPD 22.0f    
#define MAX_BL_SPD 26.0f    

typedef enum {
    BL_CUE,
    BL_SOLID,
    BL_STRIPE,
    BL_EIGHT
} BlType;

typedef enum {
    GM_START,
    GM_PLAY,
    GM_SCRATCH,
    GM_WIN,
    GM_LOSE
} GmState;

typedef enum {
    PLR_NONE,
    PLR_SOLIDS,
    PLR_STRIPES
} PlrType;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    Color col;
    BlType typ;
    int num;
    bool pktd;
    bool isStrp;
} Ball;

typedef struct {
    PlrType typ;
    int blsLeft;
    char nm[20];
} Plyr;

typedef struct {
    Ball bls[MAX_BL];
    Plyr plrs[2];
    int curPlr;
    GmState state;
    Vector2 cueBPos;    
    float pwr;             
    bool aiming;            
    bool blsMoving;
    bool frstShot;
    bool typAssigned;
    char statMsg[100];

    Vector2 dragSt;       
    float stkPullPx;  
    float stkLen;       
    bool stkRecoil;
    float recoilTmr;
} Game;


void InitGm(Game *g);
void ResetBls(Game *g);
void UpdateGm(Game *g);
void DrawGm(Game *g);
void HndlInput(Game *g);
void UpdatePhys(Game *g);
void ChkCollisions(Game *g);
void ChkPockets(Game *g);
void ChkWinCond(Game *g);
void NxtTurn(Game *g);
void ApplyScratch(Game *g);
void DrawPwrBar(Game *g);
void DrawTbl();
bool BlsMoving(Game *g);
float Dist(Vector2 a, Vector2 b);
int plrIdxForTyp(Game *g, BlType btyp);
void ElasticCollision(Ball *a, Ball *b);
void ClampBlSpd(Ball *b, float maxSpd);

void InitGm(Game *g) {

    strcpy(g->plrs[0].nm, "Player 1");
    g->plrs[0].typ = PLR_NONE;
    g->plrs[0].blsLeft = 7;

    strcpy(g->plrs[1].nm, "Player 2");
    g->plrs[1].typ = PLR_NONE;
    g->plrs[1].blsLeft = 7;

    g->curPlr = 0;
    g->state = GM_START;
    g->pwr = 0.0f;
    g->aiming = false;
    g->blsMoving = false;
    g->frstShot = true;
    g->typAssigned = false;
    strcpy(g->statMsg, "Break shot: click on cue, drag back, release to shoot");

    g->stkPullPx = 0.0f;
    g->stkLen = 120.0f;
    g->stkRecoil = false;
    g->recoilTmr = 0.0f;

    ResetBls(g);
}

void ResetBls(Game *g) {
  
    Vector2 triStart = { T_W * 0.72f, T_H * 0.5f };
    g->bls[0].pos = (Vector2){ T_W * 0.25f, T_H * 0.5f };
    g->bls[0].vel = (Vector2){0, 0};
    g->bls[0].col = WHITE;
    g->bls[0].typ = BL_CUE;
    g->bls[0].num = 0;
    g->bls[0].pktd = false;
    g->bls[0].isStrp = false;

    
    Color solidCols[] = { YELLOW, BLUE, RED, PURPLE, ORANGE, GREEN, MAROON };
    Color stripeCols[] = { YELLOW, BLUE, RED, PURPLE, ORANGE, GREEN, MAROON };

    int idx = 1;
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col <= row; col++) {
            if (idx >= MAX_BL) break;
            float offX = row * (BL_R * 2 * 0.88f);
            float offY = (col * (BL_R * 2)) - (row * BL_R);
            g->bls[idx].pos = (Vector2){ triStart.x + offX, triStart.y + offY };
            g->bls[idx].vel = (Vector2){0,0};
            g->bls[idx].pktd = false;

            if (idx == 8) {
                g->bls[idx].col = BLACK;
                g->bls[idx].typ = BL_EIGHT;
                g->bls[idx].isStrp = false;
            } else if (idx <= 7) {
                g->bls[idx].col = solidCols[idx-1];
                g->bls[idx].typ = BL_SOLID;
                g->bls[idx].isStrp = false;
            } else { // 9..15 stripes
                int sidx = idx - 9;
                if (sidx < 0) sidx = 0;
                g->bls[idx].col = stripeCols[sidx];
                g->bls[idx].typ = BL_STRIPE;
                g->bls[idx].isStrp = true;
            }
            g->bls[idx].num = idx;
            idx++;
        }
    }

   
    g->cueBPos = g->bls[0].pos;
}

void UpdateGm(Game *g) {
    
    HndlInput(g);

    
    if (g->stkRecoil) {
        g->recoilTmr -= 1.0f/60.0f;
        if (g->recoilTmr <= 0.0f) {
            g->stkRecoil = false;
            g->stkPullPx = 0.0f;
        } else {
            g->stkPullPx *= 0.92f;
            g->pwr = g->stkPullPx / MAX_PWR_PX;
            if (g->pwr < 0) g->pwr = 0;
        }
    }

    if (g->state == GM_PLAY || g->state == GM_SCRATCH) {
        UpdatePhys(g);

        if (!g->blsMoving && BlsMoving(g)) g->blsMoving = true;

        if (g->blsMoving && !BlsMoving(g)) {
            g->blsMoving = false;
            // when balls stop
            if (g->state == GM_PLAY) {
                ChkWinCond(g);
                if (g->state != GM_WIN && g->state != GM_LOSE) {
                    NxtTurn(g);
                }
            }
        }
    }
}

void DrawGm(Game *g) {
    BeginDrawing();
    ClearBackground((Color){8, 80, 23, 255}); // dark green
    DrawTbl();
    Vector2 pkts[] = {
        {RIL_W, RIL_W},
        {T_W*0.5f, RIL_W},
        {T_W - RIL_W, RIL_W},
        {RIL_W, T_H - RIL_W},
        {T_W*0.5f, T_H - RIL_W},
        {T_W - RIL_W, T_H - RIL_W}
    };

    for (int i = 0; i < 6; i++) DrawCircleV(pkts[i], PKT_R, BLACK);

    
    for (int i = 0; i < MAX_BL; i++) {
        if (!g->bls[i].pktd) {
            DrawCircleV(g->bls[i].pos, BL_R, g->bls[i].col);
            if (g->bls[i].isStrp) {
                DrawRectangleV((Vector2){g->bls[i].pos.x - BL_R*0.9f, g->bls[i].pos.y - BL_R*0.28f},
                               (Vector2){BL_R*1.8f, BL_R*0.56f}, WHITE);
                DrawCircleV(g->bls[i].pos, BL_R-1, g->bls[i].col);
            }
            if (g->bls[i].typ == BL_CUE) {
                DrawCircleV((Vector2){g->bls[i].pos.x, g->bls[i].pos.y}, 4, LIGHTGRAY);
            } else {
                char numStr[4];
                sprintf(numStr, "%d", g->bls[i].num);
                Vector2 tp = { g->bls[i].pos.x - MeasureText(numStr, 12)/2.0f, g->bls[i].pos.y - 6 };
                DrawText(numStr, tp.x, tp.y, 12, WHITE);
            }
        }
    }

    
    if ((g->state == GM_START || g->state == GM_PLAY) && !g->blsMoving) {
        Vector2 cueBPos = g->bls[0].pktd ? g->cueBPos : g->bls[0].pos;
        Vector2 mPos = GetMousePosition();
        Vector2 dir;
        float angle;
        if (g->aiming || g->stkPullPx > 0.0f) {
            // direction from cue ball to current mouse
            dir = (Vector2){ mPos.x - cueBPos.x, mPos.y - cueBPos.y };
            float len = sqrtf(dir.x*dir.x + dir.y*dir.y);
            if (len > 0.0001f) { dir.x /= len; dir.y /= len; }
            angle = atan2(dir.y, dir.x);
        } else {
            // aim toward mouse normally for preview
            dir = (Vector2){ mPos.x - cueBPos.x, mPos.y - cueBPos.y };
            float len = sqrtf(dir.x*dir.x + dir.y*dir.y);
            if (len > 0.0001f) { dir.x /= len; dir.y /= len; }
            angle = atan2(dir.y, dir.x);
        }

        float effLen = g->stkLen + g->stkPullPx;
        Vector2 stkTip = { cueBPos.x - dir.x * (BL_R + effLen),
                             cueBPos.y - dir.y * (BL_R + effLen) };
        Vector2 stkStart = { cueBPos.x - dir.x * (BL_R + 4), cueBPos.y - dir.y * (BL_R + 4) };

        DrawLineEx(stkTip, stkStart, 8.0f, (Color){100,60,20,255});
        DrawLineEx(stkTip, stkStart, 6.0f, BROWN);
        Vector2 tipPos = { stkTip.x + dir.x*6, stkTip.y + dir.y*6 };
        DrawCircleV(tipPos, 4, LIGHTGRAY);

        // aiming guide
        if (g->aiming) {
            Vector2 lineEnd = { cueBPos.x + dir.x * 420, cueBPos.y + dir.y * 420 };
            DrawLineEx(cueBPos, lineEnd, 1.5f, Fade(WHITE, 0.22f));
        }
    }

    
    DrawPwrBar(g);

    
    DrawRectangle(0, T_H, T_W, 100, (Color){30,18,10,255});
    char scoreText[128];
    sprintf(scoreText, "%s: %d balls remaining", g->plrs[0].nm, g->plrs[0].blsLeft);
    DrawText(scoreText, 18, T_H + 12, 18, WHITE);
    sprintf(scoreText, "%s: %d balls remaining", g->plrs[1].nm, g->plrs[1].blsLeft);
    DrawText(scoreText, 18, T_H + 40, 18, WHITE);

    char plrText[80];
    if (g->plrs[g->curPlr].typ == PLR_SOLIDS) sprintf(plrText, "Current: %s (Solids)", g->plrs[g->curPlr].nm);
    else if (g->plrs[g->curPlr].typ == PLR_STRIPES) sprintf(plrText, "Current: %s (Stripes)", g->plrs[g->curPlr].nm);
    else sprintf(plrText, "Current: %s (Unassigned)", g->plrs[g->curPlr].nm);

    DrawText(plrText, T_W - 360, T_H + 12, 18, WHITE);
    DrawText(g->statMsg, T_W - 360, T_H + 40, 16, YELLOW);

   
    if (g->state == GM_SCRATCH) {
        DrawRectangle(0,0,T_W,T_H+100,(Color){0,0,0,150});
        DrawText("SCRATCH! Click to place cue ball (inside rails)", T_W/2 - MeasureText("SCRATCH! Click to place cue ball (inside rails)", 20)/2,
                 T_H/2 - 10, 20, RED);
    }

    if (g->state == GM_WIN || g->state == GM_LOSE) {
        DrawRectangle(0,0,T_W,T_H+100,(Color){0,0,0,200});
        char winText[64];
        if (g->state == GM_WIN) sprintf(winText, "%s WINS!", g->plrs[g->curPlr].nm);
        else { int w = 1 - g->curPlr; sprintf(winText, "%s WINS!", g->plrs[w].nm); }
        DrawText(winText, T_W/2 - MeasureText(winText, 40)/2, T_H/2 - 40, 40, GREEN);
        DrawText("Press R to Restart", T_W/2 - MeasureText("Press R to Restart", 20)/2, T_H/2 + 10, 20, WHITE);
    }

    EndDrawing();
}

void HndlInput(Game *g) {
    
    if (IsKeyPressed(KEY_R)) {
        InitGm(g);
        return;
    }

    Vector2 mPos = GetMousePosition();

    if (g->state == GM_SCRATCH) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (mPos.x > RIL_W + BL_R && mPos.x < T_W - RIL_W - BL_R &&
                mPos.y > RIL_W + BL_R && mPos.y < T_H - RIL_W - BL_R) {
                g->cueBPos = mPos;
                g->bls[0].pos = g->cueBPos;
                g->bls[0].pktd = false;
                g->bls[0].vel = (Vector2){0,0};
                g->state = GM_PLAY;
                sprintf(g->statMsg, "Cue placed. %s's turn", g->plrs[g->curPlr].nm);
            } else {
                strcpy(g->statMsg, "Invalid position! Place inside rails away from pockets");
            }
        }
        return;
    }

    // If balls are moving, ignore aiming input
    if (g->blsMoving) return;

    // Drag-to-charge
    Vector2 cueBPos = g->bls[0].pktd ? g->cueBPos : g->bls[0].pos;

    // Start drag only if mouse pressed near cue ball
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (Dist(mPos, cueBPos) <= BL_R*1.6f) {
            g->aiming = true;
            g->dragSt = mPos;
            g->stkPullPx = 0.0f;
            g->pwr = 0.0f;
        }
    }

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && g->aiming) {
        float d = Dist(mPos, cueBPos);
        if (d > MAX_PWR_PX) d = MAX_PWR_PX;
        g->stkPullPx = d;
        g->pwr = g->stkPullPx / MAX_PWR_PX;
    }

    if (g->aiming && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        g->aiming = false;
        Vector2 relPos = mPos;
        Vector2 dir = { relPos.x - cueBPos.x, relPos.y - cueBPos.y };
        float len = sqrtf(dir.x*dir.x + dir.y*dir.y);
        if (len < 0.001f) {
            g->stkPullPx = 0.0f;
            g->pwr = 0.0f;
            return;
        }
        dir.x /= len; dir.y /= len;

        float shotSpd = (g->stkPullPx / MAX_PWR_PX) * MAX_SHT_SPD;
        if (shotSpd > MAX_SHT_SPD) shotSpd = MAX_SHT_SPD;
        g->bls[0].vel.x = dir.x * shotSpd;
        g->bls[0].vel.y = dir.y * shotSpd;

        g->state = GM_PLAY;
        g->frstShot = false;
        g->stkRecoil = true;
        g->recoilTmr = 0.12f;

        g->pwr = 0.0f;
    }
}

void UpdatePhys(Game *g) {
    for (int i = 0; i < MAX_BL; i++) {
        if (g->bls[i].pktd) continue;
       
        g->bls[i].pos.x += g->bls[i].vel.x;
        g->bls[i].pos.y += g->bls[i].vel.y;

        g->bls[i].vel.x *= FRIC;
        g->bls[i].vel.y *= FRIC;

        if (fabs(g->bls[i].vel.x) < MIN_VEL) g->bls[i].vel.x = 0;
        if (fabs(g->bls[i].vel.y) < MIN_VEL) g->bls[i].vel.y = 0;

        if (g->bls[i].pos.x - BL_R < RIL_W) {
            g->bls[i].pos.x = RIL_W + BL_R;
            g->bls[i].vel.x = -g->bls[i].vel.x * 0.86f;
        }
        if (g->bls[i].pos.x + BL_R > T_W - RIL_W) {
            g->bls[i].pos.x = T_W - RIL_W - BL_R;
            g->bls[i].vel.x = -g->bls[i].vel.x * 0.86f;
        }
        if (g->bls[i].pos.y - BL_R < RIL_W) {
            g->bls[i].pos.y = RIL_W + BL_R;
            g->bls[i].vel.y = -g->bls[i].vel.y * 0.86f;
        }
        if (g->bls[i].pos.y + BL_R > T_H - RIL_W) {
            g->bls[i].pos.y = T_H - RIL_W - BL_R;
            g->bls[i].vel.y = -g->bls[i].vel.y * 0.86f;
        }

       
        ClampBlSpd(&g->bls[i], MAX_BL_SPD);
    }

    ChkCollisions(g);
    ChkPockets(g);
}

void ChkCollisions(Game *g) {
    for (int i = 0; i < MAX_BL; i++) {
        if (g->bls[i].pktd) continue;
        for (int j = i+1; j < MAX_BL; j++) {
            if (g->bls[j].pktd) continue;

            float dist = Dist(g->bls[i].pos, g->bls[j].pos);
            float minDist = BL_R*2.0f;

            if (dist < minDist && dist > 0.0001f) {
                // resolve overlap by pushing apart equally
                float overlap = 0.5f * (minDist - dist + 0.001f);
                Vector2 norm = { (g->bls[j].pos.x - g->bls[i].pos.x) / dist,
                                   (g->bls[j].pos.y - g->bls[i].pos.y) / dist };
                g->bls[i].pos.x -= norm.x * overlap;
                g->bls[i].pos.y -= norm.y * overlap;
                g->bls[j].pos.x += norm.x * overlap;
                g->bls[j].pos.y += norm.y * overlap;

               
                ElasticCollision(&g->bls[i], &g->bls[j]);

                // clamp speeds
                ClampBlSpd(&g->bls[i], MAX_BL_SPD);
                ClampBlSpd(&g->bls[j], MAX_BL_SPD);
            }
        }
    }
}

void ElasticCollision(Ball *a, Ball *b) {
    // normal vector
    float dx = b->pos.x - a->pos.x;
    float dy = b->pos.y - a->pos.y;
    float dist = sqrtf(dx*dx + dy*dy);
    if (dist <= 0.0001f) return;
    float nx = dx / dist;
    float ny = dy / dist;

    // tangent vector
    float tx = -ny;
    float ty = nx;

    
    float va_n = a->vel.x * nx + a->vel.y * ny;
    float va_t = a->vel.x * tx + a->vel.y * ty;
    float vb_n = b->vel.x * nx + b->vel.y * ny;
    float vb_t = b->vel.x * tx + b->vel.y * ty;

    float va_n_aft = vb_n;
    float vb_n_aft = va_n;

   
    a->vel.x = va_n_aft * nx + va_t * tx;
    a->vel.y = va_n_aft * ny + va_t * ty;
    b->vel.x = vb_n_aft * nx + vb_t * tx;
    b->vel.y = vb_n_aft * ny + vb_t * ty;
}

void ClampBlSpd(Ball *b, float maxSpd) {
    float sx = b->vel.x;
    float sy = b->vel.y;
    float mag = sqrtf(sx*sx + sy*sy);
    if (mag > maxSpd) {
        b->vel.x = (b->vel.x / mag) * maxSpd;
        b->vel.y = (b->vel.y / mag) * maxSpd;
    }
}

void ChkPockets(Game *g) {
    Vector2 pkts[] = {
        {RIL_W, RIL_W},
        {T_W*0.5f, RIL_W},
        {T_W - RIL_W, RIL_W},
        {RIL_W, T_H - RIL_W},
        {T_W*0.5f, T_H - RIL_W},
        {T_W - RIL_W, T_H - RIL_W}
    };

    bool cueBPktd = false;
    bool anyPktd = false;

    for (int i = 0; i < MAX_BL; i++) {
        if (g->bls[i].pktd) continue;

        for (int p = 0; p < 6; p++) {
            if (Dist(g->bls[i].pos, pkts[p]) < PKT_R) {
                g->bls[i].pktd = true;
                g->bls[i].vel = (Vector2){0,0};
                anyPktd = true;

                if (i == 0) {
                    cueBPktd = true;
                    g->cueBPos = (Vector2){ T_W * 0.25f, T_H * 0.5f };
                } else {
                    // assign types on first pocket (break)
                    if (!g->typAssigned && g->frstShot) {
                        if (g->bls[i].typ == BL_SOLID) {
                            g->plrs[g->curPlr].typ = PLR_SOLIDS;
                            g->plrs[1 - g->curPlr].typ = PLR_STRIPES;
                            g->typAssigned = true;
                            sprintf(g->statMsg, "%s = Solids, %s = Stripes", g->plrs[g->curPlr].nm, g->plrs[1 - g->curPlr].nm);
                        } else if (g->bls[i].typ == BL_STRIPE) {
                            g->plrs[g->curPlr].typ = PLR_STRIPES;
                            g->plrs[1 - g->curPlr].typ = PLR_SOLIDS;
                            g->typAssigned = true;
                            sprintf(g->statMsg, "%s = Stripes, %s = Solids", g->plrs[g->curPlr].nm, g->plrs[1 - g->curPlr].nm);
                        }
                    }

                    // 8-ball logic
                    if (g->bls[i].typ == BL_EIGHT) {
                        int myIdx = g->curPlr;
                        if ((g->plrs[myIdx].typ == PLR_SOLIDS && g->plrs[myIdx].blsLeft == 0) ||
                            (g->plrs[myIdx].typ == PLR_STRIPES && g->plrs[myIdx].blsLeft == 0)) {
                            g->state = GM_WIN;
                        } else {
                            g->state = GM_LOSE;
                        }
                        return;
                    } else {
                        
                        int ownerIdx = plrIdxForTyp(g, g->bls[i].typ);
                        if (ownerIdx >= 0 && g->plrs[ownerIdx].blsLeft > 0) {
                            g->plrs[ownerIdx].blsLeft--;
                        }
                    }
                }
                break;
            }
        }
    }

    if (cueBPktd) ApplyScratch(g);

    if (anyPktd) {
        sprintf(g->statMsg, "%s pocketed a ball!", g->plrs[g->curPlr].nm);
    }
}

int plrIdxForTyp(Game *g, BlType btyp) {
    if (btyp == BL_SOLID) {
        if (g->plrs[0].typ == PLR_SOLIDS) return 0;
        if (g->plrs[1].typ == PLR_SOLIDS) return 1;
    } else if (btyp == BL_STRIPE) {
        if (g->plrs[0].typ == PLR_STRIPES) return 0;
        if (g->plrs[1].typ == PLR_STRIPES) return 1;
    }
    return -1;
}

void ApplyScratch(Game *g) {
    g->state = GM_SCRATCH;
    strcpy(g->statMsg, "Scratch! Place cue ball");
    
    g->curPlr = 1 - g->curPlr;
}

void ChkWinCond(Game *g) {
   
    int idx = g->curPlr;
    if (g->plrs[idx].typ == PLR_SOLIDS && g->plrs[idx].blsLeft == 0) {
        strcpy(g->statMsg, "Shoot the 8-ball!");
    } else if (g->plrs[idx].typ == PLR_STRIPES && g->plrs[idx].blsLeft == 0) {
        strcpy(g->statMsg, "Shoot the 8-ball!");
    }
}

void NxtTurn(Game *g) {
    g->curPlr = 1 - g->curPlr;
    sprintf(g->statMsg, "%s's turn", g->plrs[g->curPlr].nm);
}

void DrawPwrBar(Game *g) {
    int x = 18;
    int y = T_H + 70;
    int width = 240;
    int height = 16;

    DrawText("Power:", x, T_H + 36, 16, WHITE);
    DrawRectangle(x + 80, y, width, height, GRAY);
    int filled = (int)(width * (g->stkPullPx / MAX_PWR_PX));
    if (filled < 0) filled = 0;
    if (filled > width) filled = width;
    DrawRectangle(x + 80, y, filled, height, RED);
    DrawRectangleLines(x + 80, y, width, height, BLACK);

    char pstr[32];
    sprintf(pstr, "%d%%", (int)((g->stkPullPx / MAX_PWR_PX) * 100.0f));
    DrawText(pstr, x + 80 + width + 8, y - 2, 16, WHITE);
}

void DrawTbl() {
    DrawRectangle(RIL_W, RIL_W, T_W - 2*RIL_W, T_H - 2*RIL_W, GREEN);
    
    DrawRectangle(0, 0, T_W, RIL_W, BROWN);
    DrawRectangle(0, T_H - RIL_W, T_W, RIL_W, BROWN);
    DrawRectangle(0, 0, RIL_W, T_H, BROWN);
    DrawRectangle(T_W - RIL_W, 0, RIL_W, T_H, BROWN);
}

bool BlsMoving(Game *g) {
    for (int i = 0; i < MAX_BL; i++) {
        if (g->bls[i].pktd) continue;
        if (fabs(g->bls[i].vel.x) > MIN_VEL || fabs(g->bls[i].vel.y) > MIN_VEL) return true;
    }
    return false;
}

float Dist(Vector2 a, Vector2 b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrtf(dx*dx + dy*dy);
}

int main(void) {
    InitWindow(T_W, T_H + 100, "8 Ball Pool - Drag to Charge (Fixed)");
    SetTargetFPS(60);

    Game g;
    InitGm(&g);

    while (!WindowShouldClose()) {
        UpdateGm(&g);
        DrawGm(&g);
    }

    CloseWindow();
    return 0;
}