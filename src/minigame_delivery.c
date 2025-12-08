/**
 * ═════════════════════════════════════════════════════════════════════════════
 * ARCHIVO: src/minigame_delivery.c
 * 
 * Fase 2: Entrega - PLACEHOLDER
 * ═════════════════════════════════════════════════════════════════════════════
 */

#include "minigame_delivery.h"

#define NUM_CHIMENEAS 15
#define NUM_REGALOS_VOL 6
#define OBJETIVO_ENTREGAS 10
#define FRAME_AUTO_ENTREGA 45

typedef struct {
    Sprite* sprite;
    s16 x, y;
    u8 state; /* 0 inactiva, 1 activa, 2 completada */
} Chimenea;

typedef struct {
    Sprite* sprite;
    s16 x, y;
    s16 velX, velY;
    u8 active;
} RegaloVolador;

static Chimenea chimeneas[NUM_CHIMENEAS];
static RegaloVolador regalos[NUM_REGALOS_VOL];
static Sprite* cannonTejado;
static s16 cannonX;
static s8 cannonVelocity;
static u16 entregasCompletadas;
static u16 frameCounter;

void minigameDelivery_init(void) {
    VDP_setScreenWidth320();
    VDP_setScreenHeight224();
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);

    memset(chimeneas, 0, sizeof(chimeneas));
    memset(regalos, 0, sizeof(regalos));
    cannonTejado = NULL;
    cannonX = SCREEN_WIDTH / 2;
    cannonVelocity = 0;
    entregasCompletadas = 0;
    frameCounter = 0;

#if 0
    VDP_drawText("FASE 2 (placeholder) - Tejados", 4, 10);
    VDP_drawText("Entrega 10 regalos (simulado)", 4, 12);
#endif
}

void minigameDelivery_update(void) {
    frameCounter++;
    /* Placeholder de progreso automático */
    if ((frameCounter % FRAME_AUTO_ENTREGA) == 0 && entregasCompletadas < OBJETIVO_ENTREGAS) {
        entregasCompletadas++;
    }
}

void minigameDelivery_render(void) {
#if 0
    char buffer[32];
    sprintf(buffer, "Entregas: %u/%u", entregasCompletadas, OBJETIVO_ENTREGAS);
    VDP_drawText(buffer, 4, 14);
#endif
    SPR_update();
    SYS_doVBlankProcess();
}

u8 minigameDelivery_isComplete(void) {
    return (entregasCompletadas >= OBJETIVO_ENTREGAS);
}
