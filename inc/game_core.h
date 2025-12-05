/**
 * ═════════════════════════════════════════════════════════════════════════════
 * ARCHIVO: inc/game_core.h
 * 
 * Librería Core - Headers comunes reutilizables por todos los minijuegos
 * ═════════════════════════════════════════════════════════════════════════════
 */

#ifndef _GAME_CORE_H_
#define _GAME_CORE_H_

#include <genesis.h>
#include <kdebug.h>

/* CONSTANTES */
#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   224

/* PALETAS */
#define PAL_COMMON 0
#define PAL_PLAYER 1
#define PAL_ENEMY  2
#define PAL_EFFECT 3

/* ESTRUCTURAS */
typedef struct {
    u32 elapsed;
    u32 max_frames;
    u8 state;  // 0=RUNNING, 1=VICTORY, 2=DEFEAT
} GameTimer;

/* FUNCIONES */
u16 gameCore_readInput(void);
u8 gameCore_checkCollisionAABB(s16 x1, s16 y1, u16 w1, u16 h1,
                                s16 x2, s16 y2, u16 w2, u16 h2);
void gameCore_initTimer(GameTimer *timer, u16 seconds);
s32 gameCore_updateTimer(GameTimer *timer);
u16 gameCore_getTimeRemaining(GameTimer *timer);
void gameCore_fadeToBlack(void);
void gameCore_fadeFromBlack(void);
void gameCore_clearAllSprites(void);

#endif
