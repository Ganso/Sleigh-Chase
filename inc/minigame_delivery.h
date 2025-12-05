/**
 * ═════════════════════════════════════════════════════════════════════════════
 * ARCHIVO: inc/minigame_delivery.h
 * ═════════════════════════════════════════════════════════════════════════════
 */

#ifndef _MINIGAME_DELIVERY_H_
#define _MINIGAME_DELIVERY_H_

#include "game_core.h"

void minigameDelivery_init(void);
void minigameDelivery_update(void);
void minigameDelivery_render(void);
u8 minigameDelivery_isComplete(void);

#endif
