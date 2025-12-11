#ifndef _MINIGAME_CELEBRATION_H_
#define _MINIGAME_CELEBRATION_H_

#include "game_core.h"

/**
 * @file minigame_celebration.h
 * @brief Interfaces públicas para la fase 4: Celebración final.
 */

/** @brief Prepara recursos y limpia planos para la celebración. */
void minigameCelebration_init(void);

/** @brief Avanza la animación y contadores de celebración. */
void minigameCelebration_update(void);

/** @brief Renderiza sprites y procesa VBlank en la fase final. */
void minigameCelebration_render(void);

/**
 * @brief Informa de si el tiempo de celebración ha finalizado.
 * @return TRUE al superar la duración configurada.
 */
u8 minigameCelebration_isComplete(void);

#endif
