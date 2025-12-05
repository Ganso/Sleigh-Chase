/**
 * ═════════════════════════════════════════════════════════════════════════════
 * ARCHIVO: src/minigame_bells.c
 * 
 * Fase 3: Campanadas (100% Funcional - adaptado de main-1.c)
 * ═════════════════════════════════════════════════════════════════════════════
 */

#include "minigame_bells.h"
#include "resources.h"

/* Definiciones */
#define NUM_BELLS 6
#define NUM_FIXED_BELLS 12
#define FRAMES_BLINK 30

/* Estructura campana */
typedef struct {
    Sprite* sprite;
    s16 x, y;
    s8 velocity;
    u8 blinkCounter;
    u8 isBlinking;
} Bell;

typedef struct {
    Sprite* sprite;
    u8 active;
} FixedBell;

/* Variables locales */
static Bell bells[NUM_BELLS];
static FixedBell fixedBells[NUM_FIXED_BELLS];
static Sprite* playerCannon;
static s16 cannonX;
static s8 cannonVelocity;
static u16 bellsCompleted;
static GameTimer gameTimer;
static u16 frameCounter;

const s8 CANNON_ACCEL = 1;
const s8 CANNON_FRICTION = 1;
const s8 CANNON_MAX_VEL = 6;

/* Inicializar campana */
static void initBell(Bell* bell, u8 index) {
    s16 posX = random() % (SCREEN_WIDTH - 32);
    bell->sprite = SPR_addSpriteSafe(&sprite_campana, posX, -32,
                                      TILE_ATTR(PAL_PLAYER, FALSE, FALSE, FALSE));
    bell->x = posX;
    bell->y = -32 - (random() % 100);
    bell->velocity = (random() % 2) + 1;
    bell->blinkCounter = 0;
    bell->isBlinking = FALSE;
}

/* Reiniciar campana */
static void resetBell(Bell* bell) {
    bell->x = random() % (SCREEN_WIDTH - 32);
    bell->y = -32 - (random() % 100);
    bell->isBlinking = FALSE;
    bell->blinkCounter = 0;
    SPR_setVisibility(bell->sprite, VISIBLE);
    SPR_setPosition(bell->sprite, bell->x, bell->y);
}

/* Actualizar campana */
static void updateBell(Bell* bell) {
    /* Comprobar si llega a la línea de toque */
    if (bell->y + 32 >= SCREEN_HEIGHT - 64 && !bell->isBlinking) {
        bell->isBlinking = TRUE;
        bell->blinkCounter = FRAMES_BLINK;
    }

    /* Animación parpadeo */
    if (bell->isBlinking) {
        if (bell->blinkCounter % 2 == 0) {
            SPR_setVisibility(bell->sprite, VISIBLE);
        } else {
            SPR_setVisibility(bell->sprite, HIDDEN);
        }
        bell->blinkCounter--;

        if (bell->blinkCounter <= 0) {
            bell->isBlinking = FALSE;
            SPR_setVisibility(bell->sprite, VISIBLE);
            resetBell(bell);
        }
        return;
    }

    /* Movimiento normal */
    if ((frameCounter % bell->velocity) == 0) {
        bell->y++;
    }

    /* Reiniciar si pasa pantalla */
    if (bell->y > SCREEN_HEIGHT) {
        resetBell(bell);
    }

    SPR_setPosition(bell->sprite, bell->x, bell->y);
}

/* Inicializar campanillas fijas */
static void initFixedBells(void) {
    s16 separador_x = 24, separador_y = 16;
    s16 x = 0;
    s16 y = SCREEN_HEIGHT - 3 * separador_y - 16;

    for (u8 i = 0; i < NUM_FIXED_BELLS; i++) {
        fixedBells[i].sprite = SPR_addSprite(&sprite_campana_bn, x, y,
                                              TILE_ATTR(PAL_PLAYER, FALSE, FALSE, FALSE));
        fixedBells[i].active = FALSE;

        x += separador_x;
        if (i == 3 || i == 7) {
            y += separador_y;
            x = 0;
        }
    }
}

/* Colisión con campana */
static void handleBellCollision(Bell* bell, u8 index) {
    if (bellsCompleted < NUM_FIXED_BELLS) {
        SPR_setDefinition(fixedBells[bellsCompleted].sprite, &sprite_campana);
        fixedBells[bellsCompleted].active = TRUE;
        bellsCompleted++;
    }

    bell->isBlinking = TRUE;
    bell->blinkCounter = FRAMES_BLINK;
}

/* Inicializar minijuego */
void minigameBells_init(void) {
    /* Inicializar */
    SPR_init();
    JOY_init();
    VDP_init();

    /* Paletas */
    if (sprite_campana.palette) {
        PAL_setPalette(PAL_PLAYER, sprite_campana.palette->data, CPU);
    }

    VDP_setBackgroundColor(0);

    /* Campanas */
    for (u8 i = 0; i < NUM_BELLS; i++) {
        initBell(&bells[i], i);
    }

    initFixedBells();

    /* Cañón */
    cannonX = (SCREEN_WIDTH - 80) / 2 + 80;
    playerCannon = SPR_addSpriteSafe(&sprite_canon, cannonX,
                                      SCREEN_HEIGHT - 64,
                                      TILE_ATTR(PAL_PLAYER, TRUE, FALSE, FALSE));

    /* Timer: Sin límite para campanadas (infinito) */
    gameCore_initTimer(&gameTimer, 999);

    bellsCompleted = 0;
    frameCounter = 0;
    cannonVelocity = 0;
}

/* Actualizar minijuego */
void minigameBells_update(void) {
    u16 input = gameCore_readInput();

    /* Movimiento cañón */
    if (input & BUTTON_LEFT) {
        cannonVelocity = (cannonVelocity > -CANNON_MAX_VEL) ?
                         cannonVelocity - CANNON_ACCEL : -CANNON_MAX_VEL;
    } else if (input & BUTTON_RIGHT) {
        cannonVelocity = (cannonVelocity < CANNON_MAX_VEL) ?
                         cannonVelocity + CANNON_ACCEL : CANNON_MAX_VEL;
    } else {
        /* Fricción */
        if (cannonVelocity > 0) {
            cannonVelocity -= CANNON_FRICTION;
        } else if (cannonVelocity < 0) {
            cannonVelocity += CANNON_FRICTION;
        }
    }

    cannonX += cannonVelocity;

    /* Limites */
    if (cannonX < -32) cannonX = -32;
    if (cannonX > SCREEN_WIDTH - 32) cannonX = SCREEN_WIDTH - 32;

    SPR_setPosition(playerCannon, cannonX, SCREEN_HEIGHT - 64);

    /* Actualizar campanas */
    for (u8 i = 0; i < NUM_BELLS; i++) {
        updateBell(&bells[i]);
    }

    /* Colisión básica con botón B (sin sistema de balas) */
    if (input & BUTTON_B) {
        /* Buscar campana más baja que no está parpadeando */
        s16 lowestY = -32;
        s8 lowestBellIndex = -1;

        for (u8 i = 0; i < NUM_BELLS; i++) {
            if (!bells[i].isBlinking && bells[i].y > lowestY) {
                lowestY = bells[i].y;
                lowestBellIndex = i;
            }
        }

        if (lowestBellIndex >= 0) {
            handleBellCollision(&bells[lowestBellIndex], lowestBellIndex);
        }
    }

    gameCore_updateTimer(&gameTimer);
    frameCounter++;
}

/* Renderizar */
void minigameBells_render(void) {
    SPR_update();
    SYS_doVBlankProcess();
}

/* Comprobar victoria */
u8 minigameBells_isComplete(void) {
    return (bellsCompleted >= NUM_FIXED_BELLS);
}
