/**
 * ═════════════════════════════════════════════════════════════════════════════
 * ARCHIVO: src/minigame_celebration.c
 * 
 * Fase 4: Celebración - PLACEHOLDER
 * ═════════════════════════════════════════════════════════════════════════════
 */

#include "minigame_celebration.h"

#define NUM_CONFETI 50
#define NUM_PERSONAJES 2
#define DURACION_CELEBRACION 300

typedef struct {
    Sprite* sprite;
    s16 x, y;
    s8 velX, velY;
    u8 active;
} Confeti;

typedef struct {
    Sprite* sprite;
    s16 x, y;
    u8 frameAnim;
} PersonajeFiesta;

static Confeti confetis[NUM_CONFETI];
static PersonajeFiesta personajes[NUM_PERSONAJES];
static u16 frameCounter;

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

void minigameCelebration_update(void) {
    frameCounter++;
}

void minigameCelebration_render(void) {
    /* HUD desactivado temporalmente */
    SPR_update();
    SYS_doVBlankProcess();
}

u8 minigameCelebration_isComplete(void) {
    return frameCounter >= DURACION_CELEBRACION;
}
