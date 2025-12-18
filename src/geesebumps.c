#include <genesis.h>
#include "../res/res_geesebumps.h"
#include "audio_manager.h"
#include "game_core.h"

/**
 * @file geesebumps.c
 * @brief Presentación inicial con logo, música y fades animados.
 *
 * Recursos empleados:
 * - `res_geesebumps.h`: contiene el fondo `geesebumps_logo_bg` y las paletas
 *   `geesebumps_pal_black`, `geesebumps_pal_white`, `geesebumps_pal_white2` y
 *   `geesebumps_pal_lines`, además de los sprites `geesebumps_logo_text` y las
 *   dos líneas animadas. Cada sprite consume la paleta declarada junto a sus
 *   datos en el mismo fichero de recursos.
 * - `music_geesebumps` (también en `res_geesebumps.h`): pista reproducida en
 *   bucle durante la animación. No precisa paleta adicional al ser audio.
 */

#define SCREEN_FPS 60 /**< Frecuencia de refresco base de la Mega Drive. */

static bool should_exit = false; /**< Señal para abandonar la intro al pulsar. */

/**
 * @brief Manejador de eventos de joystick durante la pantalla de logo.
 *
 * Sale de la animación cuando se detecta un botón pulsado.
 */
static void joyEvent_Geesebumps(u16 joy, u16 changed, u16 state)
{
    /* Si se pulsa el botón principal, se abandona la animación */
    if ((changed & BUTTON_BTN) && (state & BUTTON_BTN))
    {
        should_exit = true;
    }
}

/**
 * @brief Reproduce la secuencia completa del logo con música y fades.
 *
 * Puede salir anticipadamente si se pulsa un botón.
 */
void geesebumps_logo(void)
{
    Sprite *logo_text, *logo_lines1, *logo_lines2; /**< Sprites del logo. */
    should_exit = false; /**< Reinicia la bandera de salida temprana. */

    gameCore_resetVideoState(); /**< Garantiza VRAM limpia y reinicia tile index. */
    int indice_tiles = TILE_USER_INDEX; /**< Primer tile libre para cargar BG. */
    
    /* Configura el manejador de joystick */
    JOY_setEventHandler(&joyEvent_Geesebumps);

    VDP_setBackgroundColor(13);
    PAL_setPalette(PAL0, geesebumps_pal_black.data, DMA);
    PAL_setPalette(PAL1, geesebumps_pal_white.data, DMA);
    PAL_setPalette(PAL2, geesebumps_pal_white.data, DMA);
    PAL_setPalette(PAL3, geesebumps_pal_white.data, DMA);

    /* Música de fondo */
    audio_play_intro();
    
    /* Primera parte del logo (Goose) */
    VDP_drawImageEx(BG_A, &geesebumps_logo_bg, TILE_ATTR_FULL(PAL0, false, false, false, indice_tiles), 0, 0, false, true);
    indice_tiles+=geesebumps_logo_bg.tileset->numTile;

    /* Cargar el resto de sprites */
    logo_text = SPR_addSpriteSafe(&geesebumps_logo_text, 60, 163, TILE_ATTR(PAL1, false, false, false));
    logo_lines1 = SPR_addSpriteSafe(&geesebumps_logo_line1, 81-180, 55, TILE_ATTR(PAL2, false, false, false));
    logo_lines2 = SPR_addSpriteSafe(&geesebumps_logo_line2, 81-180, 84, TILE_ATTR(PAL3, false, false, false));
    SPR_setVisibility(logo_text, HIDDEN);
    SPR_update();

    /* Segunda parte (texto) con fade in */
    if (!should_exit) {
        PAL_fade(0, 15, geesebumps_pal_black.data, geesebumps_logo_bg.palette->data, SCREEN_FPS*2, true);
                
        while (PAL_isDoingFade() && !should_exit) {
            SYS_doVBlankProcess();
        }
        PAL_setPalette(PAL0, geesebumps_logo_bg.palette->data, DMA);
        SYS_doVBlankProcess();

        if (!should_exit) {
            SPR_setVisibility(logo_text, VISIBLE);
            SPR_update();
            
            PAL_fade(16, 31, geesebumps_pal_white.data, geesebumps_logo_text.palette->data, SCREEN_FPS*2, true);
            
            while (PAL_isDoingFade() && !should_exit) {
                SYS_doVBlankProcess();
            }
            PAL_setPalette(PAL1, geesebumps_logo_text.palette->data, DMA);
            SYS_doVBlankProcess();
        }
    }

    /* Tercera y cuarta parte (líneas) con fade y scroll */
    if (!should_exit) {
        PAL_initFade(32, 63, geesebumps_pal_white2.data, geesebumps_pal_lines.data, SCREEN_FPS*3);
        for (u16 difx=180; difx>0 && !should_exit; difx--) {
            SPR_setPosition(logo_lines1, 81-difx, 55);
            SPR_setPosition(logo_lines2, 81-difx, 84);
            SPR_update();
            PAL_doFadeStep();
            SYS_doVBlankProcess();
        }
    }

    /* Pausa y fade out */
    if (!should_exit) waitMs(3000);
    PAL_fadeOutAll(SCREEN_FPS*2,false);
    XGM2_fadeOut(SCREEN_FPS*2);
    waitMs(2000);

    /* Liberar recursos */
    JOY_setEventHandler(NULL);
    VDP_releaseAllSprites();
    SPR_reset();
    VDP_clearPlane(BG_A, true);
    PAL_setPalette(PAL0, geesebumps_pal_black.data, DMA);
    PAL_setPalette(PAL1, geesebumps_pal_black.data, DMA);
    PAL_setPalette(PAL2, geesebumps_pal_black.data, DMA);
    PAL_setPalette(PAL3, geesebumps_pal_black.data, DMA);
    indice_tiles-=geesebumps_logo_bg.tileset->numTile;   
}
