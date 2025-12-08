/**
 * ═════════════════════════════════════════════════════════════════════════════
 * ARCHIVO: inc/snow_effect.h
 * 
 * Efecto de nieve en primer plano reutilizable por cualquier minijuego.
 * ═════════════════════════════════════════════════════════════════════════════
 */

#ifndef _SNOW_EFFECT_H_
#define _SNOW_EFFECT_H_

#include <genesis.h>
#include "game_core.h"

typedef struct {
    Map *map;
    s32 offsetX;
    s32 offsetY;
    s16 angle;
    s16 angleStep;
    s16 verticalStep;
    u16 widthPx;
    u16 heightPx;
} SnowEffect;

void snowEffect_init(SnowEffect *effect, u32 *tileIndex, s16 angleStep, s16 verticalStep);
void snowEffect_update(SnowEffect *effect, u16 frameCounter);

#endif
