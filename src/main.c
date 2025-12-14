/**
 * @file main.c
 * @brief Orquestador principal que encadena las cuatro fases del juego.
 *
 * Bloque de recursos por fase:
 * - Fase 1 (Recogida): usa tiles y sprites definidos en @ref resources_bg.h y
 *   @ref resources_sprites.h, con paletas extraídas de esos mismos ficheros.
 *   La paleta principal proviene de los fondos de nieve de `resources_bg.h` y
 *   se aplica a los tiles del escenario; los sprites de Santa y los elfos usan
 *   las paletas de `resources_sprites.h`.
 * - Fase 2 (Entrega): consume fondos y sprites de tejados definidos en
 *   `resources_bg.h` y `resources_sprites.h`. La paleta de tejados se declara
 *   junto a los gráficos de fondo y se comparte con los objetos interactivos.
 * - Fase 3 (Campanadas): reutiliza assets musicales de `resources_music.h` y
 *   sprites de campanas de `resources_sprites.h`; la paleta asociada a las
 *   campanas se obtiene del mismo fichero de sprites.
 * - Fase 4 (Celebración): emplea el tema musical de fin de partida definido en
 *   `resources_music.h` y sprites festivos de `resources_sprites.h`, con la
 *   paleta celebrativa declarada junto a dichos sprites.
 * - Pantalla final: solamente usa texto plano de VDP, sin recursos externos.
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
#include "cutscene.h"

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
static u8 currentPhase = PHASE_DELIVERY; /**< Fase actual del bucle principal. */

/**
 * @brief Punto de entrada principal del cartucho.
 *
 * Inicializa subsistemas SGDK y ejecuta las fases en orden secuencial.
 */
int main() {
    /* Inicialización */
    VDP_init();                     /**< Prepara el chip de vídeo para dibujar. */
    SPR_init();                     /**< Inicializa el manejador de sprites. */
    JOY_init();                     /**< Habilita lectura de mandos. */
    Z80_init();                     /**< Pone en marcha el coprocesador de sonido. */
    Z80_loadDriver(Z80_DRIVER_XGM2, 1); /**< Carga el driver de audio XGM2. */
    audio_init();                   /**< Ajusta volúmenes y modo de bucle. */

    /* Loop principal */
    while (1) {
        switch (currentPhase) {
            case PHASE_INTRO:
                /* Mostrar intro o pasar a Fase 1 */
                KLog("Mostrando intro...");
                audio_play_intro();
                geesebumps_logo();
                gameCore_fadeToBlack();
                currentPhase = PHASE_PICKUP;
                break;

            case PHASE_PICKUP:
                /* Fase 1: Recogida - Polo Norte */
                KLog("Fase 1: Recogida");
                cutscene_phase1_intro();
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
                cutscene_phase2_intro();
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
                cutscene_phase3_intro();
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
