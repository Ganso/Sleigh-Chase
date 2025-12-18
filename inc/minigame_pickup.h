#ifndef _MINIGAME_PICKUP_H_
#define _MINIGAME_PICKUP_H_

#include "game_core.h"

/**
 * @file minigame_pickup.h
 * @brief Interfaces públicas para la fase 1: Recogida de regalos.
 */

/** @brief Inicializa recursos, sprites y estado de la fase de recogida. */
void minigamePickup_init(void);

/** @brief Actualiza la lógica principal del minijuego de recogida. */
void minigamePickup_update(void);

/** @brief Renderiza sprites, HUD y efectos de la fase de recogida. */
void minigamePickup_render(void);

/**
 * @brief Indica si se alcanzó la meta de regalos de la fase.
 * @return TRUE cuando se completan los objetivos configurados.
 */
u8 minigamePickup_isComplete(void);

/** @brief Libera recursos persistentes (mapas) de la fase de recogida. */
void minigamePickup_shutdown(void);

#endif
