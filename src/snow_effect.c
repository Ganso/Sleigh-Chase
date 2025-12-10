/**
 * ═════════════════════════════════════════════════════════════════════════════
 * ARCHIVO: src/snow_effect.c
 * 
 * Implementación reutilizable del efecto de nieve en primer plano.
 * ═════════════════════════════════════════════════════════════════════════════
 */

#include "snow_effect.h"
#include "resources_bg.h"

#define SNOW_WIDTH_PX 384
#define SNOW_HEIGHT_PX 512

void snowEffect_init(SnowEffect *effect, u32 *tileIndex, s16 angleStep, s16 verticalStep) {
    if (effect == NULL || tileIndex == NULL) return;

    effect->offsetX = 0;
    effect->offsetY = 0;
    effect->angle = 0;
    effect->angleStep = 0;
    effect->verticalStep = 0;
    effect->map = NULL;
    effect->widthPx = SNOW_WIDTH_PX;
    effect->heightPx = SNOW_HEIGHT_PX;

    effect->angleStep = angleStep;
    effect->verticalStep = verticalStep;

    VDP_loadTileSet(&image_primer_plano_nieve_tile, *tileIndex, CPU);
    effect->map = MAP_create(&image_primer_plano_nieve_map, BG_A,
        TILE_ATTR_FULL(PAL_COMMON, TRUE, FALSE, FALSE, *tileIndex));
    *tileIndex += image_primer_plano_nieve_tile.numTile;

    if (effect->map != NULL) {
        MAP_scrollTo(effect->map, 0, 0);
    }
    SYS_doVBlankProcess();

}

void snowEffect_update(SnowEffect *effect, u16 frameCounter) {
    if (effect == NULL || effect->map == NULL) return;

    (void)frameCounter;
    effect->angle += effect->angleStep;
    effect->offsetX = sinFix16(effect->angle);
    effect->offsetY += effect->verticalStep;

    s32 posX = effect->offsetX % SNOW_WIDTH_PX;
    if (posX < 0) posX += SNOW_WIDTH_PX;
    s32 posY = effect->offsetY % SNOW_HEIGHT_PX;
    if (posY < 0) posY += SNOW_HEIGHT_PX;

    MAP_scrollTo(effect->map, (u32)posX, (u32)posY);
}
