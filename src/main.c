/**
 * ═════════════════════════════════════════════════════════════════════════════
 * ARCHIVO: src/main.c
 * 
 * Orquestador principal - coordina las 4 fases del juego
 * ═════════════════════════════════════════════════════════════════════════════
 */

#include <genesis.h>
#include "game_core.h"
#include "minigame_bells.h"
#include "minigame_pickup.h"
#include "minigame_delivery.h"
#include "minigame_celebration.h"
#include "geesebumps.h"
#include "audio_manager.h"
#include "resources_music.h"

/* Fases del juego */
enum {
    PHASE_INTRO = 0,
    PHASE_PICKUP = 1,
    PHASE_DELIVERY = 2,
    PHASE_BELLS = 3,
    PHASE_CELEBRATION = 4,
    PHASE_END = 5
};

/* Variables globales */
static u8 currentPhase = PHASE_INTRO;

int main() {
    /* Inicialización */
    VDP_init();
    SPR_init();
    JOY_init();
    Z80_init();
    Z80_loadDriver(Z80_DRIVER_XGM2, 1);
    audio_init();

    // Loop infinito de prueba de música (borrar en versión definitiva)
    //audio_test_playback(musica_test);

    /* Loop principal */
    while (1) {
        switch (currentPhase) {
            case PHASE_INTRO:
                /* Mostrar intro o pasar a Fase 1 */
                KLog("Mostrando intro...");
                //audio_play_intro();
                //geesebumps_logo();
                gameCore_fadeToBlack();
                currentPhase = PHASE_PICKUP;
                break;

            case PHASE_PICKUP:
                /* Fase 1: Recogida - Polo Norte */
                KLog("Fase 1: Recogida");
                minigamePickup_init();
                while (!minigamePickup_isComplete()) {
                    minigamePickup_update();
                    minigamePickup_render();
                    JOY_readJoypad(JOY_1);
                }
                gameCore_fadeToBlack();
                currentPhase = PHASE_DELIVERY;
                break;

            case PHASE_DELIVERY:
                /* Fase 2: Entrega - Tejados */
                KLog("Fase 2: Entrega");
                audio_play_phase2();
                minigameDelivery_init();
                while (!minigameDelivery_isComplete()) {
                    minigameDelivery_update();
                    minigameDelivery_render();
                    JOY_readJoypad(JOY_1);
                }
                gameCore_fadeToBlack();
                currentPhase = PHASE_BELLS;
                break;

            case PHASE_BELLS:
                /* Fase 3: Campanadas - IMPLEMENTADA */
                KLog("Fase 3: Campanadas");
                minigameBells_init();
                while (!minigameBells_isComplete()) {
                    minigameBells_update();
                    minigameBells_render();
                }
                gameCore_fadeToBlack();
                currentPhase = PHASE_CELEBRATION;
                break;

            case PHASE_CELEBRATION:
                /* Fase 4: Celebración */
                KLog("Fase 4: Celebración");
                audio_play_phase4();
                minigameCelebration_init();
                while (!minigameCelebration_isComplete()) {
                    minigameCelebration_update();
                    minigameCelebration_render();
                    JOY_readJoypad(JOY_1);
                }
                gameCore_fadeToBlack();
                currentPhase = PHASE_END;
                break;

            case PHASE_END:
                /* Mostrar pantalla final y salir */
                KLog("Mostrando pantalla final...");
                audio_stop_music();
                VDP_clearPlane(BG_A, TRUE);
                VDP_clearPlane(BG_B, TRUE);
                VDP_drawText("!FELIZ 2026!", 10, 10);
                VDP_drawText("Proyecto Navidad Mega Drive", 5, 14);
                SYS_doVBlankProcess();
                JOY_waitPress(JOY_ALL, BUTTON_ALL);
                return 0;

            default:
                break;
        }
    }

    return 0;
}
