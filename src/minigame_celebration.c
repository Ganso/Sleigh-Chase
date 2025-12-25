/**
 * @file minigame_celebration.c
 * @brief Fase 4: Celebracion final con resumen de tiempos y reinicio.
 */

#include "minigame_celebration.h"
#include "resources_sprites.h"
#include "resources_bg.h"
#include "resources_sfx.h"

#define PROMPT_BUTTONS (BUTTON_A | BUTTON_B | BUTTON_C | BUTTON_START)
#define CELEB_TIMES_START_Y 5      /* Fila base para los textos de tiempos. */

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
    gameCore_resetVideoState();
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

void minigameCelebration_shutdown(void) {
    if (celebrationMap) {
        MAP_release(celebrationMap);
        celebrationMap = NULL;
    }
}

static void loadCelebrationBackground(void) {
    gameCore_resetTileIndex();

    celebrationMap = gameCore_loadMapWithTiles(&image_fondo_fiesta_tile,
        &image_fondo_fiesta_map, BG_B, PAL_COMMON, FALSE, FALSE, FALSE,
        &globalTileIndex);
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
    static const char* const mensajesEs[] = {
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
    static const char* const mensajesEn[] = {
        "HAPPY 2026!",
        "",
        "May this new year bring you",
        "joy, health and prosperity.",
        "",
        "May all your plans and goals",
        "come true for you.",
        "",
        "Happy Holidays!",
        "",
        "",
        "GeeseBumps.com 2025",
    };

    const char* const* mensajes = (g_selectedLanguage == GAME_LANG_SPANISH) ? mensajesEs : mensajesEn;
    const u16 totalLines = (g_selectedLanguage == GAME_LANG_SPANISH) ?
        (sizeof(mensajesEs) / sizeof(mensajesEs[0])) :
        (sizeof(mensajesEn) / sizeof(mensajesEn[0]));
    const u16 screenRows = SCREEN_HEIGHT / 8;
    u16 startY = 5;

    VDP_clearPlane(BG_A, TRUE);
         
    VDP_loadFont(font.tileset, DMA);
    PAL_setPalette(PAL_EFFECT, font.palette->data, CPU);
    VDP_setTextPalette(PAL_EFFECT);

    for (u16 i = 0; i < totalLines; i++) {
        drawCenteredText(mensajes[i], startY + i, BG_A);
    }

    const char* prompt = (g_selectedLanguage == GAME_LANG_SPANISH) ?
        "} Pulsa para ver tus tiempos {" :
        "} Press to see your times {";
    drawCenteredText(prompt, startY + totalLines + 2, BG_A);
}

static void drawTimesBoard(void) {
    char buffer[40];
    VDP_clearPlane(BG_A, TRUE);

    const char* header = (g_selectedLanguage == GAME_LANG_SPANISH) ?
        "Resumen de partida" : "Run summary";
    drawCenteredText(header, CELEB_TIMES_START_Y, BG_A);

    const char* fase1Fmt = (g_selectedLanguage == GAME_LANG_SPANISH) ?
        "Fase 1: %lus" : "Stage 1: %lus";
    sprintf(buffer, fase1Fmt, (unsigned long)timePickup);
    drawCenteredText(buffer, CELEB_TIMES_START_Y + 3, BG_A);

    const char* fase2Fmt = (g_selectedLanguage == GAME_LANG_SPANISH) ?
        "Fase 2: %lus" : "Stage 2: %lus";
    sprintf(buffer, fase2Fmt, (unsigned long)timeDelivery);
    drawCenteredText(buffer, CELEB_TIMES_START_Y + 4, BG_A);

    const char* fase3Fmt = (g_selectedLanguage == GAME_LANG_SPANISH) ?
        "Fase 3: %lus" : "Stage 3: %lus";
    sprintf(buffer, fase3Fmt, (unsigned long)timeBells);
    drawCenteredText(buffer, CELEB_TIMES_START_Y + 5, BG_A);

    sprintf(buffer, "Total: %lus", (unsigned long)timeTotal);
    drawCenteredText(buffer, CELEB_TIMES_START_Y + 7, BG_A);

    const char* tip = (g_selectedLanguage == GAME_LANG_SPANISH) ?
        ">Intenta mejorar estos numeros!" :
        "Try to beat these times!";
    drawCenteredText(tip, CELEB_TIMES_START_Y + 10, BG_A);

    const char* resetPrompt = (g_selectedLanguage == GAME_LANG_SPANISH) ?
        "} Pulsa un boton para reiniciar {" :
        "} Press any button to reset {";
    drawCenteredText(resetPrompt, CELEB_TIMES_START_Y + 12, BG_A);
}
