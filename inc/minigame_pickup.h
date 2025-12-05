/**
 * ═════════════════════════════════════════════════════════════════════════════
 * ARCHIVO: inc/minigame_pickup.h
 * ═════════════════════════════════════════════════════════════════════════════
 */

#ifndef _MINIGAME_PICKUP_H_
#define _MINIGAME_PICKUP_H_

#include "game_core.h"

void minigamePickup_init(void);
void minigamePickup_update(void);
void minigamePickup_render(void);
u8 minigamePickup_isComplete(void);

#endif
