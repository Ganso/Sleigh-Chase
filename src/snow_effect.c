/**
 * ═════════════════════════════════════════════════════════════════════════════
 * ARCHIVO: src/snow_effect.c
 * 
 * Implementación reutilizable del efecto de nieve en primer plano.
 * ═════════════════════════════════════════════════════════════════════════════
 */

#include "snow_effect.h"
#include "resources_bg.h"

void snowEffect_init(SnowEffect *effect, u32 *tileIndex, s16 angleStep, s16 verticalStep) {
    if (effect == NULL || tileIndex == NULL) return;

    effect->offsetX = 0;
    effect->offsetY = 0;
    effect->angle = 0;
    effect->angleStep = 0;
    effect->verticalStep = 0;
    effect->map = NULL;
    effect->widthPx = 0;
    effect->heightPx = 0;

    effect->angleStep = angleStep;
    effect->verticalStep = verticalStep;
    effect->offsetY = verticalStep;   /* Arrancamos ya desplazados para evitar un wrap brusco inicial */

    VDP_loadTileSet(&image_primer_plano_nieve_tile, *tileIndex, CPU);
    effect->map = MAP_create(&image_primer_plano_nieve_map, BG_A,
        TILE_ATTR_FULL(PAL_COMMON, TRUE, FALSE, FALSE, *tileIndex));
    *tileIndex += image_primer_plano_nieve_tile.numTile;

    if (effect->map != NULL) {
        effect->widthPx = effect->map->w << 7;   /* 128 px por bloque */
        effect->heightPx = effect->map->h << 7;
        if (effect->widthPx == 0) effect->widthPx = SCREEN_WIDTH;
        if (effect->heightPx == 0) effect->heightPx = SCREEN_HEIGHT;

        /* Aplicamos el scroll inicial ya normalizado al rango del mapa */
        s32 wrapWidth = effect->widthPx;
        s32 wrapHeight = effect->heightPx;
        s32 posX = effect->offsetX % wrapWidth;
        if (posX < 0) posX += wrapWidth;
        s32 posY = effect->offsetY % wrapHeight;
        if (posY < 0) posY += wrapHeight;

        MAP_scrollTo(effect->map, (u32)posX, (u32)posY);
    }
}

void snowEffect_update(SnowEffect *effect, u16 frameCounter) {
    if (effect == NULL || effect->map == NULL) return;

    (void)frameCounter;
    effect->angle += effect->angleStep;
    effect->offsetX = sinFix16(effect->angle);
    effect->offsetY += effect->verticalStep;

    /* Ajustar scroll a rangos válidos del mapa (positivo y dentro del tamaño) */
    s32 wrapWidth = (effect->widthPx > 0) ? effect->widthPx : SCREEN_WIDTH;
    s32 wrapHeight = (effect->heightPx > 0) ? effect->heightPx : SCREEN_HEIGHT;

    s32 posX = effect->offsetX % wrapWidth;
    if (posX < 0) posX += wrapWidth;
    s32 posY = effect->offsetY % wrapHeight;
    if (posY < 0) posY += wrapHeight;

    MAP_scrollTo(effect->map, (u32)posX, (u32)posY);
}
