#include <genesis.h>
#include "audio_manager.h"
#include "resources_music.h"
#include "res_geesebumps.h"

/** @file audio_manager.c
 *  @brief Rutinas de conveniencia para reproducir música y efectos comunes.
 */

void audio_init(void) {
    XGM2_setLoopNumber(-1);
    XGM2_setFMVolume(80);
    XGM2_setPSGVolume(110);
}

/**
 * @brief Reproduce un track concreto en bucle con los volúmenes estándar.
 * @param track Puntero al buffer VGM a reproducir.
 */
static void audio_play_loop(const u8* track) {
    XGM2_setLoopNumber(-1);
    XGM2_setFMVolume(70);
    XGM2_setPSGVolume(100);
    XGM2_play(track);
}

void audio_play_intro(void) {
    XGM2_setLoopNumber(0);
    XGM2_play(music_geesebumps);
}

void audio_play_phase1(void) {
    audio_play_loop(musica_fondo);
}

void audio_play_phase2(void) {
    audio_play_loop(musica_fondo);
}

void audio_play_phase3(void) {
    audio_play_loop(musica_fondo);
}

void audio_play_phase4(void) {
    audio_play_loop(musica_fondo);
}

void audio_stop_music(void) {
    XGM2_stop();
}

void audio_ensure_phase1_playing(void) {
    /* Sin API para consultar estado; por si acaso reestablecemos volúmenes del loop de fase 1 */
    XGM2_setFMVolume(70);
    XGM2_setPSGVolume(100);
}

/**
 * @brief Reproduce un track concreto y bloquea el flujo para probar audio.
 * @param vgm_track Puntero al tema a reproducir.
 */
void audio_test_playback(const u8* vgm_track) {
    VDP_clearTextArea(0, 0, 40, 28);
    VDP_drawText("probando musica...", 10, 12);
    audio_play_loop(vgm_track);

    while (1) {
        SYS_doVBlankProcess();
    }
}
