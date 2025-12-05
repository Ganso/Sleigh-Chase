/**
 * ═════════════════════════════════════════════════════════════════════════════
 * ARCHIVO: inc/minigame_celebration.h
 * ═════════════════════════════════════════════════════════════════════════════
 */

#ifndef _MINIGAME_CELEBRATION_H_
#define _MINIGAME_CELEBRATION_H_

#include "game_core.h"

void minigameCelebration_init(void);
void minigameCelebration_update(void);
void minigameCelebration_render(void);
u8 minigameCelebration_isComplete(void);

#endif
