/**
 * @file minigame_delivery.c
 * @brief Fase 2: Entrega - implementación placeholder con progresión automática.
 */

#include "minigame_delivery.h"

#define NUM_CHIMENEAS 15
#define NUM_REGALOS_VOL 6
#define OBJETIVO_ENTREGAS 10
#define FRAME_AUTO_ENTREGA 45

/** @brief Entidad básica de chimenea con estado y sprite asociado. */
typedef struct {
    Sprite* sprite;
    s16 x, y;
    u8 state; /* 0 inactiva, 1 activa, 2 completada */
} Chimenea;

/** @brief Regalo volador con velocidad y estado de actividad. */
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

/** @brief Configura el escenario base de la fase de entrega. */
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

}

/**
 * @brief Actualiza contadores y avance automático del placeholder.
 *
 * Actualmente suma entregas cada cierto número de frames hasta el objetivo.
 */
void minigameDelivery_update(void) {
    frameCounter++;
    /* Placeholder de progreso automatico */
    if ((frameCounter % FRAME_AUTO_ENTREGA) == 0 && entregasCompletadas < OBJETIVO_ENTREGAS) {
        entregasCompletadas++;
    }
}

/**
 * @brief Sincroniza sprites y espera al VBlank.
 *
 * No realiza dibujo adicional mientras se implemente el placeholder.
 */
void minigameDelivery_render(void) {
    SPR_update();
    SYS_doVBlankProcess();
}

/**
 * @brief Indica si el objetivo de entregas se alcanzó.
 * @return TRUE cuando se cumplen las entregas mínimas.
 */
u8 minigameDelivery_isComplete(void) {
    return (entregasCompletadas >= OBJETIVO_ENTREGAS);
}
