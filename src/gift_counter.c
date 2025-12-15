#include "gift_counter.h"

void giftCounter_initHUD(GiftCounterHUD* hud, Sprite* top, Sprite* bottom,
    s16 baseX, s16 baseY, s16 topOffsetY, s16 bottomOffsetX,
    u16 depthTop, u16 depthBottom, u8 rowSize, u16 maxValue) {
    if (hud == NULL) return;

    hud->top = top;
    hud->bottom = bottom;
    hud->baseX = baseX;
    hud->baseY = baseY;
    hud->topOffsetY = topOffsetY;
    hud->bottomOffsetX = bottomOffsetX;
    hud->depthTop = depthTop;
    hud->depthBottom = depthBottom;
    hud->rowSize = rowSize;
    hud->maxValue = maxValue;
}

void giftCounter_startBlink(GiftCounterBlink* blink, u16 previousValue,
    u16 targetValue, u8 intervalFrames) {
    if (blink == NULL) return;
    blink->active = TRUE;
    blink->previousValue = previousValue;
    blink->targetValue = targetValue;
    blink->intervalFrames = intervalFrames ? intervalFrames : 1;
}

void giftCounter_stopBlink(GiftCounterBlink* blink) {
    if (blink == NULL) return;
    blink->active = FALSE;
    blink->previousValue = 0;
    blink->targetValue = 0;
    blink->intervalFrames = 1;
}

u16 giftCounter_getDisplayValue(const GiftCounterBlink* blink,
    u16 currentValue, u16 frameCounter) {
    if ((blink == NULL) || !blink->active) {
        return currentValue;
    }

    const u16 toggle = (frameCounter / blink->intervalFrames) & 1;
    return toggle ? blink->targetValue : blink->previousValue;
}

void giftCounter_render(const GiftCounterHUD* hud, u16 value) {
    if (hud == NULL) return;

    u16 capped = value;
    if (capped > hud->maxValue) capped = hud->maxValue;

    u16 topFrame = capped;
    if (topFrame > hud->rowSize) topFrame = hud->rowSize;

    u16 bottomFrame = 0;
    if (capped > hud->rowSize) {
        bottomFrame = capped - hud->rowSize;
        if (bottomFrame > hud->rowSize) bottomFrame = hud->rowSize;
    }

    if (hud->top) {
        SPR_setAnim(hud->top, 0);
        SPR_setFrame(hud->top, topFrame);
        SPR_setPosition(hud->top, hud->baseX, hud->baseY + hud->topOffsetY);
        SPR_setDepth(hud->top, hud->depthTop);
        SPR_setVisibility(hud->top, VISIBLE);
    }

    if (hud->bottom) {
        SPR_setAnim(hud->bottom, 0);
        SPR_setFrame(hud->bottom, bottomFrame);
        SPR_setPosition(hud->bottom, hud->baseX + hud->bottomOffsetX, hud->baseY);
        SPR_setDepth(hud->bottom, hud->depthBottom);
        SPR_setVisibility(hud->bottom, VISIBLE);
    }
}
