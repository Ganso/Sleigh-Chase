#include <genesis.h>
#include "audio_manager.h"
#include "resources.h"
#include "res_geesebumps.h"

void audio_init(void) {
    XGM2_setLoopNumber(-1);
    XGM2_setFMVolume(80);
    XGM2_setPSGVolume(110);
}

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

void audio_test_playback(const u8* vgm_track) {
    VDP_clearTextArea(0, 0, 40, 28);
    VDP_drawText("probando música", 10, 12);
    audio_play_loop(vgm_track);

    while (1) {
        SYS_doVBlankProcess();
    }
}
