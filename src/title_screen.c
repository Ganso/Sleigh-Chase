/**
 * @file title_screen.c
 * @brief Pantalla de titulo situada entre la intro y la fase de recogida.
 */

#include <genesis.h>
#include <string.h>
#include "title_screen.h"
#include "game_core.h"
#include "resources_bg.h"
#include "resources_sfx.h"
#include "audio_manager.h"
#include "resources_sprites.h"

#define TITLE_WAIT_BEFORE_SCROLL_FRAMES 60 /* 1 segundo antes de mover. */
#define TITLE_SCROLL_FRAMES 60             /* Duracion del scroll vertical. */
#define TITLE_SCROLL_START_Y 128       /* Arranca por encima de la vista. */
#define TITLE_SCROLL_TARGET_Y 0           /* Termina alineado con la pantalla. */
#define TITLE_LANGUAGE_Y 20             /* Inicio del menú. */
#define TITLE_TEXT_COLUMNS 40            /* Columnas de texto en modo 320. */
#define TITLE_TITULO_SCROLL_START_Y 0      /* Arranca visible el mapa titulo. */
#define TITLE_TITULO_SCROLL_TARGET_Y 128   /* Se desplaza fuera de la pantalla. */

static void title_draw_language_options(u8 englishSelected);

void title_show(void) {
    audio_stop_music();
    gameCore_resetVideoState();

    if (image_titulo_pal.data != NULL) {
        PAL_setPalette(PAL_PLAYER, image_titulo_pal.data, CPU);
    }
    if (image_sleigh_chase_pal.data != NULL) {
        PAL_setPalette(PAL_ENEMY, image_sleigh_chase_pal.data, CPU);
    }

    VDP_loadTileSet(&image_titulo_tile, globalTileIndex, CPU);
    Map* mapTitulo = MAP_create(&image_titulo_map, BG_B,
        TILE_ATTR_FULL(PAL_PLAYER, FALSE, FALSE, FALSE, globalTileIndex));
    globalTileIndex += image_titulo_tile.numTile;
    MAP_scrollTo(mapTitulo, 0, TITLE_TITULO_SCROLL_START_Y);

    VDP_loadTileSet(&image_sleigh_chase_tile, globalTileIndex, CPU);
    Map* mapSleigh = MAP_create(&image_sleigh_chase_map, BG_A,
        TILE_ATTR_FULL(PAL_ENEMY, FALSE, FALSE, FALSE, globalTileIndex));
    globalTileIndex += image_sleigh_chase_tile.numTile;

    MAP_scrollTo(mapSleigh, 0, TITLE_SCROLL_START_Y);

    XGM2_playPCM(snd_sleigh_chase, sizeof(snd_sleigh_chase), SOUND_PCM_CH_AUTO);

    s16 sleighScrollY = TITLE_SCROLL_START_Y;
    s16 tituloScrollY = TITLE_TITULO_SCROLL_START_Y;
    while (sleighScrollY > TITLE_SCROLL_TARGET_Y) {
        // kprintf("Haciendo map hacia (0, %d)", sleighScrollY);
        MAP_scrollTo(mapSleigh, 0, sleighScrollY);
        MAP_scrollTo(mapTitulo, 0, tituloScrollY);
        sleighScrollY -= 1;
        tituloScrollY += 1;
        SYS_doVBlankProcess();
    }
    MAP_scrollTo(mapSleigh, 0, TITLE_SCROLL_TARGET_Y);
    MAP_scrollTo(mapTitulo, 0, TITLE_TITULO_SCROLL_TARGET_Y);

    VDP_loadFont(font_dark.tileset, DMA);
    PAL_setPalette(PAL_EFFECT, font_dark.palette->data, CPU);
    VDP_setTextPalette(PAL_EFFECT);

    u8 englishSelected = TRUE;
    title_draw_language_options(englishSelected);

    u16 previousInput = 0;
    while (TRUE) {
        const u16 input = JOY_readJoypad(JOY_1);
        const u16 pressed = input & ~previousInput;

        if (pressed & (BUTTON_UP | BUTTON_DOWN)) {
            englishSelected = !englishSelected;
            title_draw_language_options(englishSelected);
        }

        if (input & (BUTTON_START | BUTTON_A | BUTTON_B | BUTTON_C)) {
            g_selectedLanguage = englishSelected ? GAME_LANG_ENGLISH : GAME_LANG_SPANISH;
            break;
        }

        previousInput = input;
        SYS_doVBlankProcess();
    }

    MAP_release(mapSleigh);
    MAP_release(mapTitulo);
    gameCore_fadeToBlack();
    gameCore_resetVideoState(); /* Limpia recursos de la intro (Sleigh Chase) antes de la fase 1. */
}

/* --- Helpers --- */

static void title_draw_language_options(u8 englishSelected) {
    const char* englishText = englishSelected ? "} ENGLISH {" : "ENGLISH";
    const char* spanishText = englishSelected ? "ESPA^OL" : "} ESPA^OL {";

    VDP_clearText(0, TITLE_LANGUAGE_Y, TITLE_TEXT_COLUMNS);
    VDP_clearText(0, TITLE_LANGUAGE_Y + 2, TITLE_TEXT_COLUMNS);

    VDP_drawText(englishText, (TITLE_TEXT_COLUMNS - strlen(englishText)) / 2, TITLE_LANGUAGE_Y);
    VDP_drawText(spanishText, (TITLE_TEXT_COLUMNS - strlen(spanishText)) / 2, TITLE_LANGUAGE_Y + 2);
}
