#ifndef _MINIGAME_BELLS_H_
#define _MINIGAME_BELLS_H_

#include "game_core.h"

/**
 * @file minigame_bells.h
 * @brief Interfaces públicas para la fase 3: Campanadas.
 */

/** @brief Inicializa recursos y estado de la fase de campanas. */
void minigameBells_init(void);

/** @brief Actualiza lógica y entrada de la fase de campanas. */
void minigameBells_update(void);

/** @brief Renderiza sprites, mapas y HUD de la fase. */
void minigameBells_render(void);

/**
 * @brief Indica si se han completado todas las campanadas.
 * @return TRUE cuando se alcanzan los objetivos de la fase.
 */
u8 minigameBells_isComplete(void);

#endif
