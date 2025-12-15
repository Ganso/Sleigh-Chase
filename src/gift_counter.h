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

void giftCounter_initHUD(GiftCounterHUD* hud, Sprite* top, Sprite* bottom,
    s16 baseX, s16 baseY, s16 topOffsetY, s16 bottomOffsetX,
    u16 depthTop, u16 depthBottom, u8 rowSize, u16 maxValue);

void giftCounter_startBlink(GiftCounterBlink* blink, u16 previousValue,
    u16 targetValue, u8 intervalFrames);

void giftCounter_stopBlink(GiftCounterBlink* blink);

u16 giftCounter_getDisplayValue(const GiftCounterBlink* blink,
    u16 currentValue, u16 frameCounter);

void giftCounter_render(const GiftCounterHUD* hud, u16 value);

#endif /* GIFT_COUNTER_H */
