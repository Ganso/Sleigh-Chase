/**
 * @file title_screen.c
 * @brief Pantalla de titulo situada entre la intro y la fase de recogida.
 */

#include <genesis.h>
#include "title_screen.h"
#include "game_core.h"
#include "resources_bg.h"
#include "resources_sfx.h"
#include "audio_manager.h"

#define TITLE_WAIT_BEFORE_SCROLL_FRAMES 60 /* 1 segundo antes de mover. */
#define TITLE_SCROLL_FRAMES 60             /* Duracion del scroll vertical. */
#define TITLE_SCROLL_START_Y 128       /* Arranca por encima de la vista. */
#define TITLE_SCROLL_TARGET_Y 0           /* Termina alineado con la pantalla. */

static void title_reset_video(void);
static u8 title_read_button(void);

void title_show(void) {
    title_reset_video();

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
    MAP_scrollTo(mapTitulo, 0, 0);

    VDP_loadTileSet(&image_sleigh_chase_tile, globalTileIndex, CPU);
    Map* map = MAP_create(&image_sleigh_chase_map, BG_A,
        TILE_ATTR_FULL(PAL_ENEMY, FALSE, FALSE, FALSE, globalTileIndex));
    globalTileIndex += image_sleigh_chase_tile.numTile;

    MAP_scrollTo(map, 0, TITLE_SCROLL_START_Y);

    for (u16 i = 0; i < TITLE_WAIT_BEFORE_SCROLL_FRAMES; i++) {
        SYS_doVBlankProcess();
    }

    XGM2_playPCM(snd_sleigh_chase, sizeof(snd_sleigh_chase), SOUND_PCM_CH_AUTO);

    s16 scrollY = TITLE_SCROLL_START_Y;
    while (scrollY > TITLE_SCROLL_TARGET_Y) {
        kprintf("Haciendo map hacia (0, %d)", scrollY);
        MAP_scrollTo(map, 0, scrollY);
        scrollY-=2;
        SYS_doVBlankProcess();
    }

    while (!title_read_button()) {
        SYS_doVBlankProcess();
    }

    gameCore_fadeToBlack();
    VDP_releaseAllSprites();
    SPR_reset();
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);
}

/* --- Helpers --- */

static void title_reset_video(void) {
    audio_stop_music();
    SPR_end();
    VDP_resetSprites();
    SPR_init();

    VDP_setScreenWidth320();
    VDP_setScreenHeight224();
    VDP_setPlaneSize(64, 64, TRUE);
    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_B, 0);
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);
    VDP_setBackgroundColor(0);
    gameCore_resetTileIndex();
    SYS_doVBlankProcess();
}

static u8 title_read_button(void) {
    const u16 input = JOY_readJoypad(JOY_1);
    return (input & (BUTTON_START | BUTTON_A | BUTTON_B | BUTTON_C)) ? TRUE : FALSE;
}
