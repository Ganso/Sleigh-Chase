#ifndef _HUD_H_
#define _HUD_H_

#include <genesis.h>

/**
 * @file hud.h
 * @brief HUD temporal que muestra contadores básicos para cada fase.
 */

/** @brief Inicializa el área del HUD limpiando la cabecera. */
void hud_init(void);

/**
 * @brief Reinicia los textos superiores según la fase activa.
 * @param phaseId Identificador de fase (1..4) o 0 para intro.
 */
void hud_reset_phase(u8 phaseId);

/**
 * @brief Dibuja el marcador de regalos recogidos y tiempo restante.
 * @param collected Regalos obtenidos.
 * @param target Objetivo de regalos para completar.
 * @param secondsRemaining Tiempo restante en segundos.
 */
void hud_draw_pickup(u16 collected, u16 target, u16 secondsRemaining);

/**
 * @brief Dibuja el marcador de entregas y tiempo restante.
 * @param delivered Entregas completadas.
 * @param target Objetivo total de entregas.
 * @param secondsRemaining Tiempo restante en segundos.
 */
void hud_draw_delivery(u16 delivered, u16 target, u16 secondsRemaining);

/**
 * @brief Dibuja el marcador de campanillas alcanzadas.
 * @param bells Campanas activadas.
 * @param target Objetivo de campanas totales.
 */
void hud_draw_bells(u16 bells, u16 target);

/** @brief Muestra el mensaje de celebración final. */
void hud_draw_celebration(void);

#endif
