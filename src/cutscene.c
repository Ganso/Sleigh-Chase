/**
 * @file cutscene.c
 * @brief Escenas previas a cada fase con fondo y texto letra a letra.
 */

#include "cutscene.h"
#include "game_core.h"
#include "resources_bg.h"
#include "resources_sprites.h"
#include "audio_manager.h"
#include <string.h>

#define CUTSCENE_TEXT_START_X 14   /* Columna inicial del texto en tiles. */
#define CUTSCENE_TEXT_START_Y 4    /* Fila inicial del texto en tiles. */
#define CUTSCENE_MAX_LINES 7       /* Máximo de líneas a mostrar en pantalla. */
#define CUTSCENE_MAX_LINE_LENGTH 24 /* Ancho máximo de cada línea en caracteres. */
#define CUTSCENE_LETTER_DELAY_FRAMES 4 /* Pausa entre letras para efecto tecleo. */

static void cutscene_play(const char* const* lines, u8 lineCount);
static u8 drawTextProgressive(const char* text, u16 x, u16 y);
static u8 waitFramesOrSkip(u16 frames);
static u8 isSkipButtonPressed(void);
static void waitSkipRelease(void);

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

void cutscene_phase1_intro(void) {
    static const char* const lines[CUTSCENE_MAX_LINES] = {
        "Los esbirros del Grinch",
        "robaron los 10 regalos",
        "que faltan por repartir",
        "    ",
        "Mis fieles ayudantes",
        "los elfos me ayudar#n",
        "a recuperarlos"
    };
    cutscene_play(lines, 7);
}

void cutscene_phase2_intro(void) {
    static const char* const lines[CUTSCENE_MAX_LINES] = {
        "Por fin puedo repartir",
        "los regalos que faltaban",
        "    ",
        "Apunta a las chimeneas",
        "que no tengan humo, y",
        "utiliza el bot*n A para",
        "lanzar los regalos"
    };
    cutscene_play(lines, 7);
}

void cutscene_phase3_intro(void) {
    static const char* const lines[CUTSCENE_MAX_LINES] = {
        "Gracias por salvar",
        "la Navidad conmigo",
        "    ",
        "Se acerca el a^o nuevo",
        "Toca las campanas y",
        "forma la felicitaci*n"
    };
    cutscene_play(lines, 6);
}

static void cutscene_play(const char* const* lines, u8 lineCount) {
    if (lines == NULL || lineCount == 0) return;

    /* Asegura estado limpio de video, sprites y audio antes de mostrar. */
    audio_stop_music();
    SPR_end();          /* Libera sprites y VRAM asociada. */
    VDP_resetSprites(); /* Limpia tabla de sprites hardware. */
    SPR_init();         /* Reinicia gestor de sprites. */
    VDP_setScreenWidth320();
    VDP_setScreenHeight224();
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);
    VDP_setPlaneSize(64, 64, TRUE);
    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_B, 0);
    VDP_setBackgroundColor(0);
    gameCore_resetTileIndex();
    SYS_doVBlankProcess();
       
    // Load font and set text palette
    VDP_loadFont(font_dark.tileset, DMA);
    PAL_setPalette(PAL_EFFECT, font_dark.palette->data, CPU);
    VDP_setTextPalette(PAL_EFFECT);

    if (image_fondo_cutscene.palette != NULL) {
        PAL_setPalette(PAL_COMMON, image_fondo_cutscene.palette->data, CPU);
    }

    VDP_drawImageEx(BG_B, &image_fondo_cutscene,
        TILE_ATTR_FULL(PAL_COMMON, FALSE, FALSE, FALSE, globalTileIndex),
        0, 0, FALSE, TRUE);
    globalTileIndex += image_fondo_cutscene.tileset->numTile;

    for (u8 i = 0; i < lineCount; i++) {
        u8 skipped = drawTextProgressive(lines[i], CUTSCENE_TEXT_START_X, CUTSCENE_TEXT_START_Y + i);
        if (skipped) {
            waitSkipRelease();
        }
    }

    const char* prompt = "} PULSA UN BOTON {";
    u16 promptX = CUTSCENE_TEXT_START_X;
    u16 promptY = CUTSCENE_TEXT_START_Y + lineCount + 2;
    u8 promptVisible = TRUE;
    u16 blinkCounter = 0;

    u16 input = 0;
    while ((input & (BUTTON_START | BUTTON_A | BUTTON_B | BUTTON_C)) == 0) {
        input = JOY_readJoypad(JOY_1);
        if (promptVisible) {
            VDP_drawText(prompt, promptX, promptY);
        } else {
            VDP_clearText(promptX, promptY, strlen(prompt));
        }
        blinkCounter++;
        if (blinkCounter >= 30) {
            blinkCounter = 0;
            promptVisible = !promptVisible;
        }
        SYS_doVBlankProcess();
    }
}

static u8 drawTextProgressive(const char* text, u16 x, u16 y) {
    if (text == NULL) return FALSE;
    char buffer[CUTSCENE_MAX_LINE_LENGTH + 1];
    u8 len = 0;

    while (text[len] != '\0' && len < CUTSCENE_MAX_LINE_LENGTH) {
        if (isSkipButtonPressed()) {
            VDP_drawText(text, x, y);
            return TRUE;
        }

        buffer[len] = text[len];
        buffer[len + 1] = '\0';
        VDP_drawText(buffer, x, y);
        if (waitFramesOrSkip(CUTSCENE_LETTER_DELAY_FRAMES)) {
            VDP_drawText(text, x, y);
            return TRUE;
        }
        len++;
    }

    return FALSE;
}

static u8 waitFramesOrSkip(u16 frames) {
    for (u16 i = 0; i < frames; i++) {
        SYS_doVBlankProcess();
        if (isSkipButtonPressed()) {
            return TRUE;
        }
    }
    return FALSE;
}

static u8 isSkipButtonPressed(void) {
    const u16 input = JOY_readJoypad(JOY_1);
    return (input & (BUTTON_START | BUTTON_A | BUTTON_B | BUTTON_C)) ? TRUE : FALSE;
}

static void waitSkipRelease(void) {
    while (isSkipButtonPressed()) {
        SYS_doVBlankProcess();
    }
}
