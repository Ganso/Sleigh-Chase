/**
 * @file game_core.c
 * @brief Implementación de funciones base reutilizables entre minijuegos.
 */

#include "game_core.h"

u32 globalTileIndex = TILE_USER_INDEX;

/** @brief Lee entrada del mando 1. */
u16 gameCore_readInput(void) {
    return JOY_readJoypad(JOY_1);
}

/**
 * @brief Inicializa un temporizador en segundos.
 * @param timer Estructura de temporizador a preparar.
 * @param seconds Duración total en segundos antes de marcar derrota.
 */
void gameCore_initTimer(GameTimer *timer, u16 seconds) {
    timer->elapsed = 0;
    timer->max_frames = seconds * 60;
    timer->state = 0;
}

/**
 * @brief Avanza el temporizador y reporta el tiempo restante.
 * @param timer Cronómetro a actualizar.
 * @return Frames restantes antes de agotar el tiempo.
 */
s32 gameCore_updateTimer(GameTimer *timer) {
    if (timer->state == 0) {
        timer->elapsed++;
        if (timer->elapsed >= timer->max_frames) {
            timer->state = 2;  /* DEFEAT por timeout */
            return 0;
        }
    }
    return timer->max_frames - timer->elapsed;
}

/**
 * @brief Fade a negro en audio y paletas.
 *
 * Usa la API corregida de SGDK para realizar el fundido y limpiar fondo.
 */
void gameCore_fadeToBlack(void) {
    XGM2_fadeOut(60);              /* Fade música */
    PAL_fadeOutAll(60, FALSE);     /* Fade paletas */
    VDP_setBackgroundColor(0);     /* Color negro */
}

/** @brief Reinicia el índice global de tiles al valor por defecto. */
void gameCore_resetTileIndex(void) {
    globalTileIndex = TILE_USER_INDEX;
}

/**
 * @brief Aplica aceleración y fricción a un eje con límites.
 *
 * @param position Posición actual a modificar.
 * @param velocity Velocidad actual a modificar.
 * @param minLimit Límite inferior permitido.
 * @param maxLimit Límite superior permitido.
 * @param inputDirection Dirección de entrada (-1,0,1).
 * @param frameIndex Índice de frame para retardos de fricción.
 * @param config Configuración de inercia a emplear.
 */
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

/**
 * @brief Aplica movimiento con inercia en los dos ejes.
 *
 * Encapsula dos llamadas a gameCore_applyInertiaAxis con límites independientes.
 */
void gameCore_applyInertiaMovement(s16 *x, s16 *y, s8 *vx, s8 *vy,
    s8 inputX, s8 inputY, s16 minX, s16 maxX, s16 minY, s16 maxY,
    u16 frameIndex, const GameInertia *config) {
    gameCore_applyInertiaAxis(x, vx, minX, maxX, inputX, frameIndex, config);
    gameCore_applyInertiaAxis(y, vy, minY, maxY, inputY, frameIndex, config);
}
