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

void gameCore_applyInertiaAxis(s16 *position, s8 *velocity, s16 minLimit, s16 maxLimit,
    s8 inputDirection, u16 frameIndex, const GameInertia *config) {
    if (config == NULL || position == NULL || velocity == NULL) return;

    if (inputDirection < 0) {
        *velocity = (*velocity > -config->maxVelocity) ?
            *velocity - config->accel : -config->maxVelocity;
    } else if (inputDirection > 0) {
        *velocity = (*velocity < config->maxVelocity) ?
            *velocity + config->accel : config->maxVelocity;
    } else {
        u16 delay = config->frictionDelay ? config->frictionDelay : 1;
        if ((frameIndex % delay) == 0) {
            if (*velocity > 0) {
                *velocity -= config->friction;
                if (*velocity < 0) *velocity = 0;
            } else if (*velocity < 0) {
                *velocity += config->friction;
                if (*velocity > 0) *velocity = 0;
            }
        }
    }

    *position += *velocity;
    if (*position < minLimit) *position = minLimit;
    if (*position > maxLimit) *position = maxLimit;
}

void gameCore_applyInertiaMovement(s16 *x, s16 *y, s8 *vx, s8 *vy,
    s8 inputX, s8 inputY, s16 minX, s16 maxX, s16 minY, s16 maxY,
    u16 frameIndex, const GameInertia *config) {
    gameCore_applyInertiaAxis(x, vx, minX, maxX, inputX, frameIndex, config);
    gameCore_applyInertiaAxis(y, vy, minY, maxY, inputY, frameIndex, config);
}
