#ifndef _AUDIO_MANAGER_H_
#define _AUDIO_MANAGER_H_

#include <genesis.h>

/**
 * @file audio_manager.h
 * @brief Declaraciones del gestor de música y efectos para todas las fases.
 */

/**
 * @brief Inicializa el subsistema de audio global.
 *
 * Configura volúmenes y bucles por defecto para que el resto de fases pueda
 * lanzar música o efectos sin preocuparse por los niveles.
 */
void audio_init(void);

/** @brief Reproduce la pista de introducción sin repetición. */
void audio_play_intro(void);

/** @brief Lanza la música en bucle para la fase 1 (recogida). */
void audio_play_phase1(void);

/** @brief Lanza la música en bucle para la fase 2 (entrega). */
void audio_play_phase2(void);

/** @brief Lanza la música en bucle para la fase 3 (campanadas). */
void audio_play_phase3(void);

/** @brief Lanza la música en bucle para la fase 4 (celebración). */
void audio_play_phase4(void);

/** @brief Detiene cualquier reproducción en curso. */
void audio_stop_music(void);

/**
 * @brief Reaplica los volúmenes del bucle de fase 1 si hubiera cambios externos.
 *
 * Útil cuando no se puede consultar el estado interno del reproductor XGM2
 * y se quiere asegurar el nivel correcto de FM/PSG.
 */
void audio_ensure_phase1_playing(void);

/**
 * @brief Reproduce un tema VGM concreto en bucle hasta reinicio.
 * @param vgm_track Puntero al buffer de datos VGM a reproducir.
 */
void audio_test_playback(const u8* vgm_track);

#endif
