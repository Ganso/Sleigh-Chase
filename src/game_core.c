/**
 * ═════════════════════════════════════════════════════════════════════════════
 * src/game_core.c
 * 
 * Implementación de funciones core (75% reutilizable)
 * CORREGIDO: API correcta de SGDK para fade
 * ═════════════════════════════════════════════════════════════════════════════
 */

#include "game_core.h"

/* Leer entrada del mando */
u16 gameCore_readInput(void) {
    return JOY_readJoypad(JOY_1);
}

/* Colisión AABB simple */
u8 gameCore_checkCollisionAABB(s16 x1, s16 y1, u16 w1, u16 h1,
                                s16 x2, s16 y2, u16 w2, u16 h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 &&
            y1 < y2 + h2 && y1 + h1 > y2);
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

/* Obtener tiempo restante en segundos */
u16 gameCore_getTimeRemaining(GameTimer *timer) {
    u32 remaining = timer->max_frames - timer->elapsed;
    return remaining / 60;
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

/**
 * Fade desde negro - Versión SGDK corregida
 * Necesita paleta cargada previamente
 */
void gameCore_fadeFromBlack(void) {
    //PAL_fadeInAll(60, FALSE);      // Fade desde negro
}

/* Limpiar sprites */
void gameCore_clearAllSprites(void) {
    SPR_reset();
}
