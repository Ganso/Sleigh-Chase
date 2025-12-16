#ifndef _MINIGAME_DELIVERY_H_
#define _MINIGAME_DELIVERY_H_

#include "game_core.h"

/**
 * @file minigame_delivery.h
 * @brief Interfaces públicas para la fase 2: Entrega en tejados.
 *
 * La fase 2 reutiliza la estructura de minijuego con movimiento libre
 * sin inercia, autoapunta la entrega al pulsar A hacia la chimenea más
 * cercana en un radio corto y mantiene el contador de regalos con
 * parpadeo al subir o bajar.
 */

/** @brief Inicializa estado y recursos del minijuego de entrega. */
void minigameDelivery_init(void);

/** @brief Procesa la lógica y progresión automática de entregas. */
void minigameDelivery_update(void);

/** @brief Dibuja sprites y sincroniza VBlank para la fase de entrega. */
void minigameDelivery_render(void);

/**
 * @brief Comprueba si se alcanzó el objetivo de entregas.
 * @return TRUE cuando se completan todas las entregas previstas.
 */
u8 minigameDelivery_isComplete(void);

#endif
