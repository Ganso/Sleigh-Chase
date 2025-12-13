/**
 * @file cutscene.c
 * @brief Escenas previas a cada fase con fondo y texto letra a letra.
 */

#include "cutscene.h"
#include "game_core.h"
#include "resources_bg.h"
#include "audio_manager.h"
#include <string.h>

#define CUTSCENE_TEXT_START_X 14   /* Columna inicial del texto en tiles. */
#define CUTSCENE_TEXT_START_Y 4    /* Fila inicial del texto en tiles. */
#define CUTSCENE_MAX_LINES 7       /* Máximo de líneas a mostrar en pantalla. */
#define CUTSCENE_MAX_LINE_LENGTH 24 /* Ancho máximo de cada línea en caracteres. */
#define CUTSCENE_LETTER_DELAY_FRAMES 4 /* Pausa entre letras para efecto tecleo. */

static void cutscene_play(const char* const* lines, u8 lineCount);
static void drawTextProgressive(const char* text, u16 x, u16 y);
static void waitFrames(u16 frames);

void cutscene_phase1_intro(void) {
    static const char* const lines[CUTSCENE_MAX_LINES] = {
        "Los elfos rojos malvados",
        "robaron los 10 regalos",
        "que faltan por repartir",
        "    ",
        "Mis fieles ayudantes",
        "los elfos verdes me",
        "ayudaran a recuperarlos"
    };
    cutscene_play(lines, 7);
}

void cutscene_phase2_intro(void) {
    static const char* const lines[CUTSCENE_MAX_LINES] = {
        "Toca repartir los",
        "10 regalos a la",
        "ciudad dormida",
        "    ",
        "Evita chimeneas con",
        "humo: estan ardiendo",
        "y no se pueden usar."
    };
    cutscene_play(lines, 6);
}

void cutscene_phase3_intro(void) {
    static const char* const lines[CUTSCENE_MAX_LINES] = {
        "Gracias por salvar",
        "la Navidad conmigo",
        "Ahora formaremos el",
        "mensaje de ano nuevo",
        "tras sonar las doce",
        "campanadas finales"
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
    
    if (image_fondo_cutscene.palette != NULL) {
        PAL_setPalette(PAL_COMMON, image_fondo_cutscene.palette->data, CPU);
    }

    VDP_drawImageEx(BG_B, &image_fondo_cutscene,
        TILE_ATTR_FULL(PAL_COMMON, FALSE, FALSE, FALSE, globalTileIndex),
        0, 0, FALSE, TRUE);
    globalTileIndex += image_fondo_cutscene.tileset->numTile;

    for (u8 i = 0; i < lineCount; i++) {
        drawTextProgressive(lines[i], CUTSCENE_TEXT_START_X, CUTSCENE_TEXT_START_Y + i);
    }

    const char* prompt = "> PULSA UN BOTON <";
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

static void drawTextProgressive(const char* text, u16 x, u16 y) {
    if (text == NULL) return;
    char buffer[CUTSCENE_MAX_LINE_LENGTH + 1];
    u8 len = 0;

    while (text[len] != '\0' && len < CUTSCENE_MAX_LINE_LENGTH) {
        buffer[len] = text[len];
        buffer[len + 1] = '\0';
        VDP_drawText(buffer, x, y);
        waitFrames(CUTSCENE_LETTER_DELAY_FRAMES);
        len++;
    }
}

static void waitFrames(u16 frames) {
    for (u16 i = 0; i < frames; i++) {
        SYS_doVBlankProcess();
    }
}
