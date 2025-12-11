#include <genesis.h>
#include "hud.h"

/** @file hud.c
 *  @brief Implementación de un HUD provisional basado en texto.
 */

static char buffer[40];

void hud_init(void) {
    VDP_clearTextArea(0, 0, 40, 3);
}

/**
 * @brief Limpia y muestra el título de la fase actual.
 * @param phaseId Identificador de fase (1..4) o 0 para intro.
 */
void hud_reset_phase(u8 phaseId) {
    VDP_clearTextArea(0, 0, 40, 3);
    switch (phaseId) {
        case 1: VDP_drawText("Fase 1: Recogida", 2, 1); break;
        case 2: VDP_drawText("Fase 2: Tejados", 2, 1); break;
        case 3: VDP_drawText("Fase 3: Campanas", 2, 1); break;
        case 4: VDP_drawText("Fase 4: Fiesta", 2, 1); break;
        default: VDP_drawText("Intro", 2, 1); break;
    }
}

/**
 * @brief Muestra progreso de regalos recogidos y tiempo restante.
 * @param collected Regalos recogidos.
 * @param target Objetivo total de regalos.
 * @param secondsRemaining Tiempo restante en segundos.
 */
void hud_draw_pickup(u16 collected, u16 target, u16 secondsRemaining) {
    sprintf(buffer, "Regalos: %u/%u  Tiempo:%us", collected, target, secondsRemaining);
    VDP_drawText(buffer, 2, 2);
}

/**
 * @brief Muestra progreso de entregas y tiempo restante.
 * @param delivered Entregas completadas.
 * @param target Objetivo total de entregas.
 * @param secondsRemaining Tiempo restante en segundos.
 */
void hud_draw_delivery(u16 delivered, u16 target, u16 secondsRemaining) {
    sprintf(buffer, "Entregas: %u/%u  Tiempo:%us", delivered, target, secondsRemaining);
    VDP_drawText(buffer, 2, 2);
}

/**
 * @brief Muestra progreso de campanillas activadas.
 * @param bells Campanas activadas.
 * @param target Objetivo total de campanas.
 */
void hud_draw_bells(u16 bells, u16 target) {
    sprintf(buffer, "Campanillas: %u/%u", bells, target);
    VDP_drawText(buffer, 2, 2);
}

/** @brief Muestra el mensaje estático de celebración. */
void hud_draw_celebration(void) {
    VDP_drawText("Celebrando... FELIZ 2026!", 2, 2);
}
