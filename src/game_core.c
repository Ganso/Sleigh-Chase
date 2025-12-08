/**
 * ═════════════════════════════════════════════════════════════════════════════
 * src/game_core.c
 * 
 * Implementación de funciones core (75% reutilizable)
 * CORREGIDO: API correcta de SGDK para fade
 * ═════════════════════════════════════════════════════════════════════════════
 */

#include "game_core.h"

u32 globalTileIndex = TILE_USER_INDEX;

/* Leer entrada del mando */
u16 gameCore_readInput(void) {
    return JOY_readJoypad(JOY_1);
}

/* Inicializar timer */
void gameCore_initTimer(GameTimer *timer, u16 seconds) {
    timer->elapsed = 0;
    timer->max_frames = seconds * 60;
    timer->state = 0;
}

/* Actualizar timer cada frame */
s32 gameCore_updateTimer(GameTimer *timer) {
    if (timer->state == 0) {
        timer->elapsed++;
        if (timer->elapsed >= timer->max_frames) {
            timer->state = 2;  // DEFEAT por timeout
            return 0;
        }
    }
    return timer->max_frames - timer->elapsed;
}

/**
 * Fade a negro - Versión SGDK corregida
 * PAL_fadeOutAll(steps, async)
 */
void gameCore_fadeToBlack(void) {
    XGM2_fadeOut(60);              // Fade música
    PAL_fadeOutAll(60, FALSE);     // Fade paletas
    VDP_setBackgroundColor(0);     // Color negro
}

void gameCore_resetTileIndex(void) {
    globalTileIndex = TILE_USER_INDEX;
}
