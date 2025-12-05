/**
 * ═════════════════════════════════════════════════════════════════════════════
 * ARCHIVO: inc/minigame_bells.h
 * ═════════════════════════════════════════════════════════════════════════════
 */

#ifndef _MINIGAME_BELLS_H_
#define _MINIGAME_BELLS_H_

#include "game_core.h"

void minigameBells_init(void);
void minigameBells_update(void);
void minigameBells_render(void);
u8 minigameBells_isComplete(void);

#endif
