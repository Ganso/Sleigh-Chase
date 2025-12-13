/**
 * @file snow_effect.c
 * @brief Implementación reutilizable del efecto de nieve en primer plano.
 */

#include "snow_effect.h"
#include "resources_bg.h"

#define SNOW_WIDTH_PX 384
#define SNOW_HEIGHT_PX 512

/**
 * @brief Inicializa el mapa de nieve y sus parámetros de movimiento.
 * @param effect Estructura a preparar.
 * @param tileIndex Índice global de tiles; se incrementa tras cargar el tileset.
 * @param angleStep Incremento de ángulo por frame para el desplazamiento sinusoidal.
 * @param verticalStep Desplazamiento vertical por frame.
 */
void snowEffect_init(SnowEffect *effect, u32 *tileIndex, s16 angleStep, s16 verticalStep) {
    if (effect == NULL || tileIndex == NULL) return;

    effect->offsetX = 0;           /**< Desplazamiento horizontal inicial. */
    effect->offsetY = 0;           /**< Desplazamiento vertical inicial. */
    effect->angle = 0;             /**< Ángulo para el movimiento sinusoidal. */
    effect->angleStep = 0;         /**< Se rellenará con el paso deseado. */
    effect->verticalStep = 0;      /**< Se rellenará con la velocidad vertical. */
    effect->map = NULL;            /**< El mapa se asignará tras cargar tiles. */
    effect->widthPx = SNOW_WIDTH_PX; /**< Ancho del patrón de nieve. */
    effect->heightPx = SNOW_HEIGHT_PX; /**< Alto del patrón de nieve. */

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

/**
 * @brief Actualiza el scroll de nieve en función del tiempo.
 * @param effect Estructura inicializada con snowEffect_init.
 * @param frameCounter Contador de frames que gobierna la animación.
 */
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
