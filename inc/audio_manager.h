/**
 * Gestor de audio placeholder para todas las fases.
 * Centraliza llamadas de música/SFX para rellenar más adelante.
 */

#ifndef _AUDIO_MANAGER_H_
#define _AUDIO_MANAGER_H_

#include <genesis.h>

void audio_init(void);
void audio_play_intro(void);
void audio_play_phase1(void);
void audio_play_phase2(void);
void audio_play_phase3(void);
void audio_play_phase4(void);
void audio_stop_music(void);
void audio_test_playback(const u8* vgm_track);

#endif
