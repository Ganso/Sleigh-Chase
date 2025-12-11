#ifndef _SNOW_EFFECT_H_
#define _SNOW_EFFECT_H_

#include <genesis.h>
#include "game_core.h"

/**
 * @file snow_effect.h
 * @brief Efecto de nieve en primer plano reutilizable por cualquier minijuego.
 */

/**
 * @brief Estado interno del efecto de nieve animada.
 */
typedef struct {
    Map *map;         /**< Mapa de tiles usado para el scroll. */
    s32 offsetX;      /**< Desfase horizontal aplicado al mapa. */
    s32 offsetY;      /**< Desfase vertical aplicado al mapa. */
    s16 angle;        /**< Ángulo para el movimiento sinusoidal. */
    s16 angleStep;    /**< Incremento de ángulo por frame. */
    s16 verticalStep; /**< Incremento de desplazamiento vertical por frame. */
    u16 widthPx;      /**< Ancho en píxeles del mapa de nieve. */
    u16 heightPx;     /**< Alto en píxeles del mapa de nieve. */
} SnowEffect;

/**
 * @brief Inicializa recursos y parámetros del efecto de nieve.
 * @param effect Estructura a rellenar.
 * @param tileIndex Índice global de tiles; se actualiza tras cargar el tileset.
 * @param angleStep Incremento de ángulo horizontal por frame.
 * @param verticalStep Incremento vertical aplicado cada frame.
 */
void snowEffect_init(SnowEffect *effect, u32 *tileIndex, s16 angleStep, s16 verticalStep);

/**
 * @brief Actualiza desplazamientos y scroll del mapa de nieve.
 * @param effect Estructura previamente inicializada.
 * @param frameCounter Contador de frames usado para animaciones dependientes del tiempo.
 */
void snowEffect_update(SnowEffect *effect, u16 frameCounter);

#endif
