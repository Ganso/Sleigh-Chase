#ifndef _MINIGAME_CELEBRATION_H_
#define _MINIGAME_CELEBRATION_H_

#include "game_core.h"

/**
 * @file minigame_celebration.h
 * @brief Interfaces publicas para la fase 4: Celebracion final.
 */

/**
 * @brief Registra los tiempos por fase para mostrarlos en la celebracion.
 * @param pickup Segundos consumidos en la fase de recogida.
 * @param delivery Segundos consumidos en la fase de entrega.
 * @param bells Segundos consumidos en la fase de campanadas.
 */
void minigameCelebration_setTimes(u32 pickup, u32 delivery, u32 bells);

/** @brief Prepara recursos y limpia planos para la celebracion. */
void minigameCelebration_init(void);

/** @brief Avanza la animacion y contadores de celebracion. */
void minigameCelebration_update(void);

/** @brief Renderiza sprites y procesa VBlank en la fase final. */
void minigameCelebration_render(void);

/**
 * @brief Informa de si el flujo de celebracion ha terminado.
 * @return TRUE cuando se dispara el reinicio.
 */
u8 minigameCelebration_isComplete(void);

#endif
