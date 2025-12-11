/**
 * @file minigame_celebration.c
 * @brief Fase 4: Celebración - placeholder con temporizador fijo.
 */

#include "minigame_celebration.h"

#define NUM_CONFETI 50
#define NUM_PERSONAJES 2
#define DURACION_CELEBRACION 300

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

static Confeti confetis[NUM_CONFETI];
static PersonajeFiesta personajes[NUM_PERSONAJES];
static u16 frameCounter;

/** @brief Prepara sprites y limpia el escenario para la celebración. */
void minigameCelebration_init(void) {
    VDP_setScreenWidth320();
    VDP_setScreenHeight224();
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);

    memset(confetis, 0, sizeof(confetis));
    memset(personajes, 0, sizeof(personajes));
    frameCounter = 0;

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
