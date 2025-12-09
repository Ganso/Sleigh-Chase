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

/* DEBUG GLOBAL (1=ON, 0=OFF) */
#ifndef DEBUG_MODE
#define DEBUG_MODE 1
#endif

/* PALETAS */
#define PAL_COMMON 0
#define PAL_PLAYER 1
#define PAL_ENEMY  2
#define PAL_EFFECT 3

typedef struct {
    s8 accel;
    s8 friction;
    u8 frictionDelay; /* aplica friccion cada N frames; 0/1 = cada frame */
    s8 maxVelocity;
} GameInertia;

/* TILESET INDEX GLOBAL */
extern u32 globalTileIndex;
void gameCore_resetTileIndex(void);

/* ESTRUCTURAS */
typedef struct {
    u32 elapsed;
    u32 max_frames;
    u8 state;  // 0=RUNNING, 1=VICTORY, 2=DEFEAT
} GameTimer;

/* FUNCIONES */
u16 gameCore_readInput(void);
void gameCore_initTimer(GameTimer *timer, u16 seconds);
s32 gameCore_updateTimer(GameTimer *timer);
void gameCore_fadeToBlack(void);
void gameCore_applyInertiaAxis(s16 *position, s8 *velocity, s16 minLimit, s16 maxLimit,
    s8 inputDirection, u16 frameIndex, const GameInertia *config);
void gameCore_applyInertiaMovement(s16 *x, s16 *y, s8 *vx, s8 *vy,
    s8 inputX, s8 inputY, s16 minX, s16 maxX, s16 minY, s16 maxY,
    u16 frameIndex, const GameInertia *config);

#endif
