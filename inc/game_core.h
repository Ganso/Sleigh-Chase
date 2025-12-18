#ifndef _GAME_CORE_H_
#define _GAME_CORE_H_

#include <genesis.h>
#include <kdebug.h>

/**
 * @file game_core.h
 * @brief Utilidades y definiciones comunes reutilizadas por todos los minijuegos.
 */

/* CONSTANTES */
#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   224

/* DEBUG GLOBAL */
// #ifndef DEBUG_MODE
// #define DEBUG_MODE
// #endif

/* PALETAS */
#define PAL_COMMON 0
#define PAL_PLAYER 1
#define PAL_ENEMY  2
#define PAL_EFFECT 3

/**
 * @brief Configuración genérica para movimiento con inercia.
 */
typedef struct {
    s8 accel;            /**< Incremento de velocidad por frame con entrada. */
    s8 friction;         /**< Reducción de velocidad cuando no hay entrada. */
    u8 frictionDelay;    /**< Cada cuántos frames aplicar fricción (0/1 = cada frame). */
    s8 maxVelocity;      /**< Límite absoluto de velocidad por eje. */
} GameInertia;

/* TILESET INDEX GLOBAL */
extern u32 globalTileIndex;

/**
 * @brief Idiomas disponibles para los textos.
 */
typedef enum {
    GAME_LANG_ENGLISH = 0, /**< Inglés. */
    GAME_LANG_SPANISH = 1  /**< Español. */
} GameLanguage;

/**
 * @brief Idioma seleccionado globalmente.
 */
extern GameLanguage g_selectedLanguage;

/** @brief Reinicia el índice global de tiles al valor por defecto del usuario. */
void gameCore_resetTileIndex(void);

/**
 * @brief Cronómetro simple para fases con o sin límite de tiempo.
 */
typedef struct {
    u32 elapsed;       /**< Frames transcurridos desde el inicio. */
    u32 max_frames;    /**< Frames de referencia (0 = sin límite). */
    u8 state;          /**< 0=RUNNING, 1=VICTORY, 2=DEFEAT. */
} GameTimer;

/* FUNCIONES */

/**
 * @brief Lee el estado del mando 1.
 * @return Máscara de botones según JOY_readJoypad.
 */
u16 gameCore_readInput(void);

/**
 * @brief Inicializa un temporizador en segundos.
 * @param timer Estructura a rellenar.
 * @param seconds Tiempo de referencia en segundos (0 = sin límite).
 */
void gameCore_initTimer(GameTimer *timer, u16 seconds);

/**
 * @brief Avanza el temporizador y devuelve el tiempo restante o transcurrido.
 * @param timer Cronómetro a actualizar.
 * @return Frames restantes si hay límite; frames transcurridos si max_frames es 0.
 */
s32 gameCore_updateTimer(GameTimer *timer);

/**
 * @brief Realiza un fundido a negro tanto en paletas como en audio.
 */
void gameCore_fadeToBlack(void);

/**
 * @brief Aplica inercia y fricción a un eje con límites.
 *
 * @param position Posición actual a actualizar.
 * @param velocity Velocidad actual a actualizar.
 * @param minLimit Límite mínimo permitido.
 * @param maxLimit Límite máximo permitido.
 * @param inputDirection Dirección de entrada (-1,0,1).
 * @param frameIndex Índice de frame para calcular retardos de fricción.
 * @param config Configuración de inercia a aplicar.
 */
void gameCore_applyInertiaAxis(s16 *position, s8 *velocity, s16 minLimit, s16 maxLimit,
    s8 inputDirection, u16 frameIndex, const GameInertia *config);

/**
 * @brief Aplica movimiento con inercia en los dos ejes principales.
 *
 * Encapsula dos llamadas a gameCore_applyInertiaAxis limitando X/Y por separado.
 */
void gameCore_applyInertiaMovement(s16 *x, s16 *y, s8 *vx, s8 *vy,
    s8 inputX, s8 inputY, s16 minX, s16 maxX, s16 minY, s16 maxY,
    u16 frameIndex, const GameInertia *config);

/**
 * @brief Comprueba el solapamiento de dos AABB.
 */
u8 gameCore_checkCollision(s16 x1, s16 y1, s16 w1, s16 h1, s16 x2, s16 y2, s16 w2, s16 h2);

#endif
