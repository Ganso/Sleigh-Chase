/**
 * @file minigame_celebration.c
 * @brief Fase 4: Celebración - placeholder con temporizador fijo.
 *
 * Recursos actuales:
 * - No se cargan fondos ni paletas específicas; la fase limpia ambos planos y
 *   reutiliza la configuración de VDP por defecto. Los sprites de confeti y
 *   personajes están pendientes de asignar desde `resources_sprites.h` cuando
 *   se implementen, por lo que este módulo actúa como esqueleto documentado.
 */

#include "minigame_celebration.h"

#define NUM_CONFETI 50           /* Número de partículas de confeti simuladas. */
#define NUM_PERSONAJES 2         /* Personajes decorativos en pantalla. */
#define DURACION_CELEBRACION 300 /* Duración total de la fase en frames. */

/** @brief Partícula de confeti simple. */
typedef struct {
    Sprite* sprite;
    s16 x, y;
    s8 velX, velY;
    u8 active;
} Confeti;

/** @brief Personaje decorativo durante la celebración. */
typedef struct {
    Sprite* sprite;
    s16 x, y;
    u8 frameAnim;
} PersonajeFiesta;

static Confeti confetis[NUM_CONFETI]; /**< Pool de partículas de confeti. */
static PersonajeFiesta personajes[NUM_PERSONAJES]; /**< Personajes decorativos. */
static u16 frameCounter; /**< Cronómetro de duración de la celebración. */

/** @brief Prepara sprites y limpia el escenario para la celebración. */
void minigameCelebration_init(void) {
    VDP_setScreenWidth320();
    VDP_setScreenHeight224();
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);

    memset(confetis, 0, sizeof(confetis));
    memset(personajes, 0, sizeof(personajes));
    frameCounter = 0; /**< Se reinicia el cronómetro al iniciar la fase. */

    /* HUD desactivado temporalmente */
}

/** @brief Avanza el contador de celebración. */
void minigameCelebration_update(void) {
    frameCounter++;
}

/** @brief Actualiza sprites y procesa VBlank. */
void minigameCelebration_render(void) {
    /* HUD desactivado temporalmente */
    SPR_update();
    SYS_doVBlankProcess();
}

/**
 * @brief Devuelve si la duración predefinida ha concluido.
 * @return TRUE cuando se supera DURACION_CELEBRACION.
 */
u8 minigameCelebration_isComplete(void) {
    return frameCounter >= DURACION_CELEBRACION;
}
