/**
 * ═════════════════════════════════════════════════════════════════════════════
 * ARCHIVO: src/minigame_pickup.c
 * 
 * Fase 1: Recogida - PLACEHOLDER (estructura lista para implementar)
 * ═════════════════════════════════════════════════════════════════════════════
 */

#include "minigame_pickup.h"

void minigamePickup_init(void) {
    /* TODO: Implementar Fase 1 */
    /* Estructura base */
}

void minigamePickup_update(void) {
    /* TODO: Lógica Fase 1 */
}

void minigamePickup_render(void) {
    SPR_update();
    SYS_doVBlankProcess();
}

u8 minigamePickup_isComplete(void) {
    /* TODO: Condición victoria */
    return true;
}

