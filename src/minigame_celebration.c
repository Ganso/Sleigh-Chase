/**
 * @file minigame_celebration.c
 * @brief Fase 4: Celebracion final con resumen de tiempos y reinicio.
 */

#include "minigame_celebration.h"
#include "resources_sprites.h"
#include "resources_bg.h"

#define PROMPT_BUTTONS (BUTTON_A | BUTTON_B | BUTTON_C | BUTTON_START)

typedef enum {
    CELEB_STATE_MESSAGE = 0,
    CELEB_STATE_TIMES,
    CELEB_STATE_RESETTING,
} CelebrationState;

static CelebrationState celebrationState;
static Map* celebrationMap;
static u16 prevInput;

static u32 timePickup;
static u32 timeDelivery;
static u32 timeBells;
static u32 timeTotal;

static void loadCelebrationBackground(void);
static void drawCenteredText(const char* text, u16 y, VDPPlane plane);
static void drawVictoryMessage(void);
static void drawTimesBoard(void);

void minigameCelebration_setTimes(u32 pickup, u32 delivery, u32 bells) {
    timePickup = pickup;
    timeDelivery = delivery;
    timeBells = bells;
    timeTotal = pickup + delivery + bells;
}

void minigameCelebration_init(void) {
    VDP_setScreenWidth320();
    VDP_setScreenHeight224();
    VDP_setBackgroundColor(0);
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);

    loadCelebrationBackground();
    drawVictoryMessage();

    celebrationState = CELEB_STATE_MESSAGE;
    prevInput = 0;
}

void minigameCelebration_update(void) {
    u16 input = gameCore_readInput();
    u16 pressed = input & ~prevInput;
    prevInput = input;

    if (celebrationState == CELEB_STATE_MESSAGE) {
        if (pressed & PROMPT_BUTTONS) {
            drawTimesBoard();
            celebrationState = CELEB_STATE_TIMES;
        }
    } else if (celebrationState == CELEB_STATE_TIMES) {
        if (pressed & PROMPT_BUTTONS) {
            celebrationState = CELEB_STATE_RESETTING;
            SYS_hardReset();
        }
    }
}

void minigameCelebration_render(void) {
    SYS_doVBlankProcess();
}

u8 minigameCelebration_isComplete(void) {
    return celebrationState == CELEB_STATE_RESETTING;
}

static void loadCelebrationBackground(void) {
    gameCore_resetTileIndex();

    VDP_loadTileSet(&image_fondo_fiesta_tile, globalTileIndex, CPU);
    celebrationMap = MAP_create(&image_fondo_fiesta_map, BG_B,
        TILE_ATTR_FULL(PAL_COMMON, FALSE, FALSE, FALSE, globalTileIndex));
    globalTileIndex += image_fondo_tile.numTile;
    PAL_setPalette(PAL_COMMON, image_fondo_fiesta_pal.data, CPU);
    MAP_scrollTo(celebrationMap, 0, 0);
    (void)celebrationMap;
}

static void drawCenteredText(const char* text, u16 y, VDPPlane plane) {
    if (!text) return;
    u16 len = strlen(text);
    u16 x = (len < 40) ? (40 - len) / 2 : 0;
    VDP_drawTextBG(plane, text, x, y);
}

// SPANISH CHARSET
// ñ --> ^
// á --> #
// é --> $
// í --> %
// ó --> *
// ú --> /
// < --> ¿
// > --> ¡
// {} --> Flechas para remarcar

static void drawVictoryMessage(void) {
    static const char* mensajes[] = {
        ">FELIZ 2026!",
        "",
        "Que este nuevo a^o te traiga",
        "alegria, salud y prosperidad.",
        "",
        "Que todos tus planes y metas",
        "se hagan realidad.",
        "",
        ">Felices Fiestas!",
        "",
        "",
        "GeeseBumps.com 2025",
    };

    const u16 totalLines = sizeof(mensajes) / sizeof(mensajes[0]);
    const u16 screenRows = SCREEN_HEIGHT / 8;
    u16 startY = 5;

    VDP_clearPlane(BG_A, TRUE);
         
    VDP_loadFont(font.tileset, DMA);
    PAL_setPalette(PAL_EFFECT, font.palette->data, CPU);
    VDP_setTextPalette(PAL_EFFECT);

    for (u16 i = 0; i < totalLines; i++) {
        drawCenteredText(mensajes[i], startY + i, BG_A);
    }

    drawCenteredText("} Pulsa para ver tus tiempos {", startY + totalLines + 2, BG_A);
}

static void drawTimesBoard(void) {
    char buffer[40];
    VDP_clearPlane(BG_A, TRUE);

    drawCenteredText("Resumen de partida", 4, BG_A);

    sprintf(buffer, "Fase 1: %lus", (unsigned long)timePickup);
    drawCenteredText(buffer, 7, BG_A);

    sprintf(buffer, "Fase 2: %lus", (unsigned long)timeDelivery);
    drawCenteredText(buffer, 8, BG_A);

    sprintf(buffer, "Fase 3: %lus", (unsigned long)timeBells);
    drawCenteredText(buffer, 9, BG_A);

    sprintf(buffer, "Total: %lus", (unsigned long)timeTotal);
    drawCenteredText(buffer, 11, BG_A);

    drawCenteredText(">Intenta mejorar estos numeros!", 14, BG_A);
    drawCenteredText("} Pulsa un boton para reiniciar {", 16, BG_A);
}
