#ifndef GIFT_COUNTER_H
#define GIFT_COUNTER_H

#include <genesis.h>

/** @brief Configuración visual para el contador de regalos. */
typedef struct {
    Sprite* top;       /**< Sprite de la fila superior. */
    Sprite* bottom;    /**< Sprite de la fila inferior. */
    s16 baseX;         /**< Posición base X (origen de la fila superior). */
    s16 baseY;         /**< Posición base Y (origen de la fila inferior). */
    s16 topOffsetY;    /**< Desplazamiento vertical aplicado a la fila superior. */
    s16 bottomOffsetX; /**< Desplazamiento horizontal aplicado a la fila inferior. */
    u16 depthTop;      /**< Profundidad del sprite superior. */
    u16 depthBottom;   /**< Profundidad del sprite inferior. */
    u8 rowSize;        /**< Número de regalos por fila. */
    u16 maxValue;      /**< Valor máximo mostrado por el contador. */
} GiftCounterHUD;

/** @brief Estado de parpadeo para transiciones del contador. */
typedef struct {
    u8 active;           /**< TRUE cuando el parpadeo está habilitado. */
    u16 previousValue;   /**< Valor anterior que alterna. */
    u16 targetValue;     /**< Valor final que alterna. */
    u8 intervalFrames;   /**< Intervalo de parpadeo en frames. */
} GiftCounterBlink;

/**
 * @brief Configura el HUD del contador de regalos.
 *
 * @param hud Estructura del HUD a rellenar.
 * @param top Sprite para la fila superior.
 * @param bottom Sprite para la fila inferior.
 * @param baseX Posición base X del HUD.
 * @param baseY Posición base Y del HUD.
 * @param topOffsetY Desplazamiento vertical de la fila superior.
 * @param bottomOffsetX Desplazamiento horizontal de la fila inferior.
 * @param depthTop Profundidad del sprite superior.
 * @param depthBottom Profundidad del sprite inferior.
 * @param rowSize Número de regalos por fila.
 * @param maxValue Valor máximo representable.
 */
void giftCounter_initHUD(GiftCounterHUD* hud, Sprite* top, Sprite* bottom,
    s16 baseX, s16 baseY, s16 topOffsetY, s16 bottomOffsetX,
    u16 depthTop, u16 depthBottom, u8 rowSize, u16 maxValue);

/**
 * @brief Inicia el parpadeo de transición entre dos valores.
 *
 * @param blink Estado de parpadeo a preparar.
 * @param previousValue Valor previo que alternará con el final.
 * @param targetValue Valor destino para el parpadeo.
 * @param intervalFrames Intervalo en frames entre cambios de valor.
 */
void giftCounter_startBlink(GiftCounterBlink* blink, u16 previousValue,
    u16 targetValue, u8 intervalFrames);

/**
 * @brief Detiene el parpadeo y reinicia sus parámetros.
 * @param blink Estado de parpadeo a limpiar.
 */
void giftCounter_stopBlink(GiftCounterBlink* blink);

/**
 * @brief Devuelve el valor actual a mostrar según el parpadeo.
 *
 * @param blink Estado de parpadeo activo o NULL.
 * @param currentValue Valor real del contador.
 * @param frameCounter Contador global de frames.
 * @return Valor alternado si hay parpadeo, o el valor real.
 */
u16 giftCounter_getDisplayValue(const GiftCounterBlink* blink,
    u16 currentValue, u16 frameCounter);

/**
 * @brief Renderiza los sprites del contador con el valor dado.
 *
 * @param hud Configuración del HUD.
 * @param value Valor a representar.
 */
void giftCounter_render(const GiftCounterHUD* hud, u16 value);

#endif /* GIFT_COUNTER_H */
