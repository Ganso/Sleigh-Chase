/**
 * ═════════════════════════════════════════════════════════════════════════════
 * ARCHIVO: src/minigame_pickup.c
 *
 * Fase 1: Recogida - Paseo cenital con pista de nieve y scroll vertical.
 * Estado actual: mecanicas acotadas; falta disparo especial definitivo y sprite de regalo lateral.
 * ═════════════════════════════════════════════════════════════════════════════
 */

#include "minigame_pickup.h"
#include "resources_bg.h"
#include "resources_sprites.h"
#include "snow_effect.h"

#define NUM_TREES 1
#define NUM_ELVES  2
#define NUM_ENEMIES 2
#define GIFTS_FOR_SPECIAL 3
#define TARGET_GIFTS 15
#define SCROLL_SPEED 1
#define FORBIDDEN_PERCENT 10

#define ENEMY_LATERAL_DELAY 30
#define ENEMY_LATERAL_SPEED 1

#define TREE_SIZE 64
#define TREE_HITBOX_HEIGHT 10
#define ENEMY_SIZE 32
#define ENEMY_HITBOX_HEIGHT 10
#define ELF_SIZE 32
#define ELF_MARK_SIZE 16
#define ELF_MARK_VISIBLE_MIN_Y 150       /* y inferior mínima para mostrar X (arriba) */
#define ELF_MARK_VISIBLE_MAX_Y 210      /* y inferior máxima para mostrar X (abajo) */
#define ELF_MARK_SCREEN_MARGIN_PERCENT 20
#define ELF_SHADOW_MIN_DEPTH SPR_MAX_DEPTH
#define ELF_RESPAWN_DELAY_MIN_FRAMES 60   /* 1s a 60fps */
#define ELF_RESPAWN_DELAY_MAX_FRAMES 300  /* 5s a 60fps */
#define GIFT_SIZE 32
#define GIFT_ARC_HEIGHT 50
#define SANTA_WIDTH 80
#define SANTA_HEIGHT 128
#define SANTA_HITBOX_PADDING 30   /* píxeles que no colisionan a cada lado */
#define SANTA_HITBOX_WIDTH (SANTA_WIDTH - 2 * SANTA_HITBOX_PADDING)
#define SANTA_VERTICAL_SPEED 2
#define TRACK_HEIGHT_PX 640
#define VERTICAL_SLOW_DIV 2  /* factor de retardo vertical (50%) */

typedef struct {
    Sprite* sprite;
    s16 x, y;
    u8 active;
} SimpleActor;

typedef struct {
    Sprite* sprite;
    s16 x, y;
    s8 vx;
    s8 vy;
    u8 specialReady;
} Santa;

static Santa santa;
static SimpleActor trees[NUM_TREES];
static SimpleActor elves[NUM_ELVES];
static SimpleActor enemies[NUM_ENEMIES];
static s16 elfSpawnY[NUM_ELVES];
static u8 elfMarkShown[NUM_ELVES];
static Sprite* elfMarkSprites[NUM_ELVES];
static s16 elfMarkPosX[NUM_ELVES];
static s16 elfMarkPosY[NUM_ELVES];
static Sprite* elfShadowSprites[NUM_ELVES];
static s16 elfShadowStartX[NUM_ELVES];
static s16 elfShadowStartY[NUM_ELVES];
static u8 elfShadowActive[NUM_ELVES];
static s16 elfShadowPosX[NUM_ELVES];
static s16 elfShadowPosY[NUM_ELVES];
static Sprite* elfGiftSprites[NUM_ELVES];
static u8 elfGiftActive[NUM_ELVES];
static u16 elfRespawnTimer[NUM_ELVES];
static u8 elfSide[NUM_ELVES];
static Map *mapTrack;
static s16 trackHeightPx;
static s16 trackOffsetY;
static s16 trackLoopLength;
static u16 giftsCollected;
static u16 giftsCharge;
static u16 frameCounter;

static s16 leftLimit;
static s16 rightLimit;
static s16 playableWidth;
static s16 santaMinY;
static s16 santaMaxY;
static GameInertia santaInertia;
static SnowEffect snowEffect;

static u8 validateHorizontalRange(s16 minX, s16 maxX, const char* context) {
    if (maxX <= minX) {
        kprintf("[%s] Rango X invalido (min=%d, max=%d)", context, minX, maxX);
        return FALSE;
    }
    return TRUE;
}

static void markActorInactive(SimpleActor *actor, const char* context) {
    actor->active = FALSE;
    kprintf("[%s] Actor desactivado por error de inicializacion", context);
    if (actor->sprite != NULL) {
        SPR_setVisibility(actor->sprite, HIDDEN);
    }
}

static void placeActor(SimpleActor *actor, s16 minX, s16 maxX, s16 minY, s16 maxY) {
    actor->x = minX + (random() % (maxX - minX));
    actor->y = -((random() % (maxY - minY)) + minY);
    actor->active = TRUE;
}

static u16 randomFrameDelay(u16 minFrames, u16 maxFrames) {
    if (maxFrames <= minFrames) return minFrames;
    return minFrames + (random() % (maxFrames - minFrames + 1));
}

static void hideElfMark(u8 index) {
    elfMarkShown[index] = FALSE;
    if (elfMarkSprites[index]) {
        SPR_setVisibility(elfMarkSprites[index], HIDDEN);
    }
    kprintf("[ELF %d] Marca X oculta", index);
}

static s16 randomPositionWithMargin(s16 totalSize, s16 elementSize, u8 marginPercent) {
    s16 margin = (totalSize * marginPercent) / 100;
    s16 min = margin;
    s16 max = totalSize - margin - elementSize;
    if (max < min) {
        min = 0;
        max = totalSize - elementSize;
    }
    if (max < 0) {
        return 0;
    }
    s16 range = max - min;
    if (range <= 0) {
        return min;
    }
    return min + (random() % (range + 1));
}

static void showElfMark(u8 index) {
    s16 posX = randomPositionWithMargin(SCREEN_WIDTH, ELF_MARK_SIZE, ELF_MARK_SCREEN_MARGIN_PERCENT);
    s16 posY = randomPositionWithMargin(SCREEN_HEIGHT, ELF_MARK_SIZE, ELF_MARK_SCREEN_MARGIN_PERCENT);
    elfMarkPosX[index] = posX;
    elfMarkPosY[index] = posY;

    if (elfMarkSprites[index] == NULL) {
        elfMarkSprites[index] = SPR_addSpriteSafe(&sprite_marca_x, posX, posY,
            TILE_ATTR(PAL_PLAYER, FALSE, FALSE, FALSE));
    }

    if (elfMarkSprites[index]) {
        SPR_setVisibility(elfMarkSprites[index], VISIBLE);
        SPR_setPosition(elfMarkSprites[index], posX, posY);
        SPR_setDepth(elfMarkSprites[index], SPR_MAX_DEPTH); /* siempre al fondo */
        elfMarkShown[index] = TRUE;
        kprintf("[ELF %d] Marca X mostrada en (%d,%d)", index, posX, posY);
    }
}

static void spawnTree(SimpleActor *tree) {
    s16 minX = leftLimit;
    s16 maxX = rightLimit - TREE_SIZE;
    if (!validateHorizontalRange(minX, maxX, "TREE")) {
        markActorInactive(tree, "TREE");
        return;
    }

    placeActor(tree, minX, maxX, 40, SCREEN_HEIGHT);
    if (tree->sprite == NULL) {
        tree->sprite = SPR_addSpriteSafe(&sprite_arbol_pista, tree->x, tree->y,
            TILE_ATTR(PAL_COMMON, FALSE, FALSE, FALSE));
    }
    if (tree->sprite == NULL) {
        markActorInactive(tree, "TREE");
        return;
    }
    tree->active = TRUE;
    SPR_setPosition(tree->sprite, tree->x, tree->y);
}

static void spawnElf(SimpleActor *elf, u8 side, u8 index) {
    elf->x = (side == 0) ? (leftLimit - ELF_SIZE - 5) : (rightLimit + 5);
    elf->y = -((random() % (SCREEN_HEIGHT - 60)) + 60);
    elf->active = TRUE;
    elfSpawnY[index] = elf->y;
    hideElfMark(index);
    elfShadowActive[index] = FALSE;
    if (elfShadowSprites[index]) {
        SPR_setVisibility(elfShadowSprites[index], HIDDEN);
    }
    elfGiftActive[index] = FALSE;
    if (elfGiftSprites[index]) {
        SPR_setVisibility(elfGiftSprites[index], HIDDEN);
    }
    kprintf("[ELF %d] Aparece en y=%d", index, elf->y);

    if (elf->sprite == NULL) {
        elf->sprite = SPR_addSpriteSafe(&sprite_elfo_lateral, elf->x, elf->y,
            TILE_ATTR(PAL_PLAYER, FALSE, FALSE, FALSE));
    }
    if (elf->sprite == NULL) {
        markActorInactive(elf, "ELF");
        return;
    }
    elf->active = TRUE;
    SPR_setHFlip(elf->sprite, side == 1);
    SPR_setPosition(elf->sprite, elf->x, elf->y);
    SPR_setVisibility(elf->sprite, VISIBLE);
}

static void spawnEnemy(SimpleActor *enemy) {
    s16 minX = leftLimit;
    s16 maxX = rightLimit - ENEMY_SIZE;
    if (!validateHorizontalRange(minX, maxX, "ENEMY")) {
        markActorInactive(enemy, "ENEMY");
        return;
    }

    placeActor(enemy, minX, maxX, 30, SCREEN_HEIGHT);
    if (enemy->sprite == NULL) {
        enemy->sprite = SPR_addSpriteSafe(&sprite_duende_malo, enemy->x, enemy->y,
            TILE_ATTR(PAL_EFFECT, FALSE, FALSE, FALSE));
    }
    if (enemy->sprite == NULL) {
        markActorInactive(enemy, "ENEMY");
        return;
    }
    enemy->active = TRUE;
    SPR_setPosition(enemy->sprite, enemy->x, enemy->y);
}

static void hideElfShadow(u8 index) {
    elfShadowActive[index] = FALSE;
    if (elfShadowSprites[index]) {
        SPR_setVisibility(elfShadowSprites[index], HIDDEN);
    }
    elfShadowPosX[index] = elfShadowStartX[index];
    elfShadowPosY[index] = elfShadowStartY[index];
}

static void showElfShadow(u8 index, s16 startX, s16 startY) {
    elfShadowStartX[index] = startX;
    elfShadowStartY[index] = startY;
    elfShadowActive[index] = TRUE;
    elfShadowPosX[index] = startX;
    elfShadowPosY[index] = startY;

    if (elfShadowSprites[index] == NULL) {
        elfShadowSprites[index] = SPR_addSpriteSafe(&sprite_sombra_regalo, startX, startY,
            TILE_ATTR(PAL_EFFECT, FALSE, FALSE, FALSE));
    }
    if (elfShadowSprites[index]) {
        SPR_setVisibility(elfShadowSprites[index], VISIBLE);
        SPR_setDepth(elfShadowSprites[index], ELF_SHADOW_MIN_DEPTH);
        SPR_setPosition(elfShadowSprites[index], startX, startY);
        elfShadowPosX[index] = startX;
        elfShadowPosY[index] = startY;
        kprintf("[ELF %d] Sombra iniciada en (%d,%d)", index, startX, startY);
    } else {
        elfShadowActive[index] = FALSE;
    }
}

static void hideElfGift(u8 index) {
    elfGiftActive[index] = FALSE;
    if (elfGiftSprites[index]) {
        SPR_setVisibility(elfGiftSprites[index], HIDDEN);
    }
}

static void showElfGift(u8 index, s16 startX, s16 startY) {
    elfGiftActive[index] = TRUE;
    if (elfGiftSprites[index] == NULL) {
        elfGiftSprites[index] = SPR_addSpriteSafe(&sprite_regalo, startX, startY,
            TILE_ATTR(PAL_EFFECT, FALSE, FALSE, FALSE));
    }
    if (elfGiftSprites[index]) {
        SPR_setVisibility(elfGiftSprites[index], VISIBLE);
        SPR_setPosition(elfGiftSprites[index], startX, startY);
    } else {
        elfGiftActive[index] = FALSE;
    }
}

static void updateElfGift(u8 index, fix16 progress) {
    if (!elfGiftActive[index] || elfGiftSprites[index] == NULL) {
        return;
    }

    if (progress <= FIX16(0)) {
        SPR_setPosition(elfGiftSprites[index], elfShadowStartX[index], elfShadowStartY[index]);
        return;
    }

    const s16 dx = elfMarkPosX[index] - elfShadowStartX[index];
    const s16 dy = elfMarkPosY[index] - elfShadowStartY[index];
    fix16 baseXf = FIX16(elfShadowStartX[index]) + F16_mul(FIX16(dx), progress);
    fix16 baseYf = FIX16(elfShadowStartY[index]) + F16_mul(FIX16(dy), progress);

    /* Arco parabólico: altura máxima GIFT_ARC_HEIGHT en t=0.5 */
    fix16 t = progress;
    fix16 arcFactor = F16_mul(FIX16(4), F16_mul(t, (FIX16(1) - t)));
    fix16 arcOffsetF = F16_mul(FIX16(GIFT_ARC_HEIGHT), arcFactor);

    s16 posX = F16_toInt(baseXf + FIX16(0.5));
    s16 posY = F16_toInt((baseYf - arcOffsetF) + FIX16(0.5));
    SPR_setPosition(elfGiftSprites[index], posX, posY);
}

static void scheduleElfRespawn(u8 index, u8 side) {
    elfSide[index] = side;
    elfRespawnTimer[index] = randomFrameDelay(ELF_RESPAWN_DELAY_MIN_FRAMES, ELF_RESPAWN_DELAY_MAX_FRAMES);
    elves[index].active = FALSE;
    if (elves[index].sprite) SPR_setVisibility(elves[index].sprite, HIDDEN);
    hideElfMark(index);
    hideElfShadow(index);
    hideElfGift(index);
    kprintf("[ELF %d] Respawn en %u frames", index, elfRespawnTimer[index]);
}

static void updateElfShadow(u8 index, fix16 progress) {
    if (!elfShadowActive[index] || elfShadowSprites[index] == NULL) {
        return;
    }
    if (progress <= FIX16(0)) {
        SPR_setPosition(elfShadowSprites[index], elfShadowStartX[index], elfShadowStartY[index]);
        elfShadowPosX[index] = elfShadowStartX[index];
        elfShadowPosY[index] = elfShadowStartY[index];
        return;
    }
    if (progress >= FIX16(1)) {
        SPR_setPosition(elfShadowSprites[index], elfMarkPosX[index], elfMarkPosY[index]);
        elfShadowPosX[index] = elfMarkPosX[index];
        elfShadowPosY[index] = elfMarkPosY[index];
        return;
    }

    const s16 dx = elfMarkPosX[index] - elfShadowStartX[index];
    const s16 dy = elfMarkPosY[index] - elfShadowStartY[index];
    fix16 offsetX = F16_mul(FIX16(dx), progress);
    fix16 offsetY = F16_mul(FIX16(dy), progress);
    s16 newX = elfShadowStartX[index] + F16_toInt(offsetX + FIX16(0.5));
    s16 newY = elfShadowStartY[index] + F16_toInt(offsetY + FIX16(0.5));
    SPR_setPosition(elfShadowSprites[index], newX, newY);
    elfShadowPosX[index] = newX;
    elfShadowPosY[index] = newY;
}

static void updateElfMark(u8 index) {
    const s16 elfBottom = elves[index].y + ELF_SIZE;
    kprintf("[ELF %d] bottomY=%d", index, elfBottom);

    if (!elves[index].active) {
        hideElfMark(index);
        hideElfShadow(index);
        hideElfGift(index);
        if (elfRespawnTimer[index] > 0) {
            elfRespawnTimer[index]--;
            if (elfRespawnTimer[index] == 0) {
                spawnElf(&elves[index], elfSide[index], index);
            }
        }
        return;
    }

    const s16 minVisibleY = ELF_MARK_VISIBLE_MIN_Y;
    const s16 maxVisibleY = ELF_MARK_VISIBLE_MAX_Y;
    const u8 belowRange = elfBottom < minVisibleY;
    const u8 aboveRange = elfBottom > maxVisibleY;

    if (belowRange) {
        hideElfMark(index);
        hideElfShadow(index);
        hideElfGift(index);
        return;
    }

    /* Muestra solo dentro del rango visible */
    if (!elfMarkShown[index]) {
        showElfMark(index);
        showElfShadow(index, elves[index].x, elves[index].y);
        showElfGift(index, elves[index].x, elves[index].y);
    }

    /* Progreso lineal del viaje de la sombra basado en la posición del elfo en la franja */
    const s16 travelSpan = maxVisibleY - minVisibleY;
    fix16 progress = FIX16(0);
    if (travelSpan > 0) {
        progress = FIX16(elfBottom - minVisibleY);
        progress = F16_div(progress, FIX16(travelSpan));
    }
    if (progress < FIX16(0)) progress = FIX16(0);
    if (progress > FIX16(1)) progress = FIX16(1);

    updateElfShadow(index, progress);
    updateElfGift(index, progress);

    if (aboveRange) {
        hideElfMark(index);
        hideElfShadow(index);
        hideElfGift(index);
    }
}

static void resetSpecialIfReady(void) {
    if (giftsCharge >= GIFTS_FOR_SPECIAL) {
        santa.specialReady = TRUE;
        /* Animación alternativa desactivada para evitar errores si no existe */
        SPR_setAnim(santa.sprite, 0);
    }
}

static void clampTrackOffset(void) {
    if (trackLoopLength <= 0) {
        trackOffsetY = 0;
        return;
    }

    while (trackOffsetY < 0) {
        trackOffsetY += trackLoopLength;
    }
    while (trackOffsetY >= trackLoopLength) {
        trackOffsetY -= trackLoopLength;
    }
}

static void collectGift(void) {
    giftsCollected++;
    giftsCharge++;
    resetSpecialIfReady();
}

static void clearEnemies(void) {
    for (u8 i = 0; i < NUM_ENEMIES; i++) {
        spawnEnemy(&enemies[i]);
    }
}

static u8 checkCollision(s16 x1, s16 y1, s16 w1, s16 h1, s16 x2, s16 y2, s16 w2, s16 h2) {
    return (x1 < x2 + w2) && (x1 + w1 > x2) && (y1 < y2 + h2) && (y1 + h1 > y2);
}

typedef struct {
    Sprite* sprite;
    s16 bottom;
} DepthEntry;

static void reorderDepthByBottom(void) {
    DepthEntry entries[NUM_TREES + NUM_ENEMIES + NUM_ELVES + 1];
    u8 count = 0;

    if (santa.sprite) {
        entries[count].sprite = santa.sprite;
        entries[count].bottom = santa.y + SANTA_HEIGHT;
        count++;
    }

    for (u8 i = 0; i < NUM_TREES; i++) {
        if (trees[i].sprite && trees[i].active) {
            entries[count].sprite = trees[i].sprite;
            entries[count].bottom = trees[i].y + TREE_SIZE;
            count++;
        }
    }

    for (u8 i = 0; i < NUM_ENEMIES; i++) {
        if (enemies[i].sprite && enemies[i].active) {
            entries[count].sprite = enemies[i].sprite;
            entries[count].bottom = enemies[i].y + ENEMY_SIZE;
            count++;
        }
    }

    for (u8 i = 0; i < NUM_ELVES; i++) {
        if (elfGiftSprites[i] && elfGiftActive[i]) {
            entries[count].sprite = elfGiftSprites[i];
            entries[count].bottom = elfShadowPosY[i] + GIFT_SIZE;
            count++;
        }
    }

    /* Ordenar por bottom descendente (el que esta mas abajo va delante) */
    for (u8 i = 0; i < count; i++) {
        for (u8 j = i + 1; j < count; j++) {
            if (entries[j].bottom > entries[i].bottom) {
                DepthEntry tmp = entries[i];
                entries[i] = entries[j];
                entries[j] = tmp;
            }
        }
    }

    u16 depth = SPR_MIN_DEPTH;
    for (u8 i = 0; i < count; i++) {
        SPR_setDepth(entries[i].sprite, depth++);
    }
}

#if DEBUG_MODE
static void renderDebug(void) {
}
#endif

void minigamePickup_init(void) {
    VDP_setScreenWidth320();
    VDP_setScreenHeight224();
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);

    gameCore_resetTileIndex();
    trackOffsetY = 0;
    trackHeightPx = 0;
    trackLoopLength = 0;
    giftsCollected = 0;
    giftsCharge = 0;
    frameCounter = 0;

    leftLimit = (SCREEN_WIDTH * FORBIDDEN_PERCENT) / 100;
    rightLimit = SCREEN_WIDTH - leftLimit;
    playableWidth = rightLimit - leftLimit - SANTA_WIDTH;
    santaMinY = 0;
    santaMaxY = SCREEN_HEIGHT - (SANTA_HEIGHT / 2);
    santaInertia.accel = 1;
    santaInertia.friction = 1;
    santaInertia.frictionDelay = 3; /* frena cada 3 frames para aumentar la inercia */
    santaInertia.maxVelocity = 4;

    if (image_pista_polo_pal.data) {
        PAL_setPalette(PAL_COMMON, image_pista_polo_pal.data, CPU);
    }
    if (sprite_santa_car.palette) {
        PAL_setPalette(PAL_PLAYER, sprite_santa_car.palette->data, CPU);
    }
    if (sprite_duende_malo.palette) {
        PAL_setPalette(PAL_EFFECT, sprite_duende_malo.palette->data, CPU);
    }

    VDP_setBackgroundColor(0);

    VDP_loadTileSet(&image_pista_polo_tile, globalTileIndex, CPU);
    mapTrack = MAP_create(&image_pista_polo_map, BG_B,
        TILE_ATTR_FULL(PAL_COMMON, FALSE, FALSE, FALSE, globalTileIndex));
    globalTileIndex += image_pista_polo_tile.numTile;

    trackHeightPx = (image_pista_polo_map.h > 0)
        ? (image_pista_polo_map.h << 3) /* 8 px por tile */
        : SCREEN_HEIGHT;
    if (trackHeightPx < SCREEN_HEIGHT) trackHeightPx = SCREEN_HEIGHT;

    trackLoopLength = (trackHeightPx > SCREEN_HEIGHT)
        ? (trackHeightPx - SCREEN_HEIGHT)
        : SCREEN_HEIGHT;
    trackOffsetY = trackLoopLength - 1;
    if (mapTrack != NULL) {
        const s16 initialOffset = trackOffsetY % trackHeightPx;
        MAP_scrollTo(mapTrack, 0, initialOffset);
        //VDP_setVerticalScroll(BG_B, initialOffset);
    } else {
        trackHeightPx = 0;
        trackLoopLength = 0;
        trackOffsetY = 0;
    }

    snowEffect_init(&snowEffect, &globalTileIndex, 1, -2);

    santa.x = leftLimit + playableWidth / 2;
    santa.y = santaMaxY;
    santa.vx = 0;
    santa.vy = 0;
    santa.specialReady = FALSE;
    santa.sprite = SPR_addSpriteSafe(&sprite_santa_car, santa.x, santa.y,
        TILE_ATTR(PAL_PLAYER, FALSE, FALSE, FALSE));
    SPR_setAnim(santa.sprite, 0);
    SPR_setDepth(santa.sprite, SPR_MIN_DEPTH);

    for (u8 i = 0; i < NUM_TREES; i++) {
        trees[i].sprite = NULL;
        spawnTree(&trees[i]);
    }
    for (u8 i = 0; i < NUM_ELVES; i++) {
        elves[i].sprite = NULL;
        elfMarkSprites[i] = NULL;
        elfShadowSprites[i] = NULL;
        elfShadowActive[i] = FALSE;
        elfMarkPosX[i] = 0;
        elfMarkPosY[i] = 0;
        elfShadowPosX[i] = 0;
        elfShadowPosY[i] = 0;
        elfGiftSprites[i] = NULL;
        elfGiftActive[i] = FALSE;
        elfRespawnTimer[i] = randomFrameDelay(ELF_RESPAWN_DELAY_MIN_FRAMES, ELF_RESPAWN_DELAY_MAX_FRAMES);
        elfSide[i] = i % 2;
        elves[i].active = FALSE;
        kprintf("[ELF %d] Respawn inicial en %u frames", i, elfRespawnTimer[i]);
    }
    for (u8 i = 0; i < NUM_ENEMIES; i++) {
        enemies[i].sprite = NULL;
        spawnEnemy(&enemies[i]);
    }
}

void minigamePickup_update(void) {
    u16 input = gameCore_readInput();
    s8 inputDirX = 0;
    s8 inputDirY = 0;

    if (input & BUTTON_LEFT) inputDirX = -1;
    else if (input & BUTTON_RIGHT) inputDirX = 1;

    if (input & BUTTON_UP) inputDirY = -1;
    else if (input & BUTTON_DOWN) inputDirY = 1;

    if (santa.specialReady && (input & BUTTON_B)) {
        santa.specialReady = FALSE;
        giftsCharge = 0;
        SPR_setAnim(santa.sprite, 0);
        clearEnemies();
    }

    gameCore_applyInertiaMovement(&santa.x, &santa.y, &santa.vx, &santa.vy,
        inputDirX, inputDirY,
        leftLimit, leftLimit + playableWidth,
        santaMinY, santaMaxY,
        frameCounter, &santaInertia);
    SPR_setPosition(santa.sprite, santa.x, santa.y);

    /* Caja de colisión reducida: solo los 40 px centrales (80 px de sprite) */
    const s16 santaHitX = santa.x + SANTA_HITBOX_PADDING;
    const s16 santaHitY = santa.y;
    const s16 santaHitW = SANTA_HITBOX_WIDTH;
    const s16 santaHitH = SANTA_HEIGHT;

    /* Scroll del fondo hacia abajo (caida) */
    const s16 scrollStep = ((frameCounter % VERTICAL_SLOW_DIV) == 0) ? SCROLL_SPEED : 0;
    if (scrollStep) {
        trackOffsetY -= scrollStep;
        clampTrackOffset(); /* envuelve dentro del rango visible del mapa */
        if (mapTrack != NULL) {
            MAP_scrollTo(mapTrack, 0, trackOffsetY);
            //VDP_setVerticalScroll(BG_B, trackOffsetY);
        }
    }

    snowEffect_update(&snowEffect, frameCounter);

    for (u8 i = 0; i < NUM_TREES; i++) {
        if (!trees[i].active) continue;
        if (scrollStep) {
            trees[i].y += scrollStep;
            if (trees[i].y > SCREEN_HEIGHT) {
                spawnTree(&trees[i]);
            }
        }
        if (checkCollision(
                santaHitX, santaHitY, santaHitW, santaHitH,
                trees[i].x,
                trees[i].y + (TREE_SIZE - TREE_HITBOX_HEIGHT),
                TREE_SIZE, TREE_HITBOX_HEIGHT)) {
            collectGift();
            spawnTree(&trees[i]);
        }
        SPR_setPosition(trees[i].sprite, trees[i].x, trees[i].y);
    }

    for (u8 i = 0; i < NUM_ELVES; i++) {
        if (!elves[i].active) {
            hideElfMark(i);
            hideElfShadow(i);
            hideElfGift(i);
            if (elfRespawnTimer[i] > 0) {
                elfRespawnTimer[i]--;
                if (elfRespawnTimer[i] == 0) {
                    spawnElf(&elves[i], elfSide[i], i);
                }
            }
            continue;
        }
        if (scrollStep) {
            elves[i].y += scrollStep;
            if (elves[i].y > SCREEN_HEIGHT) {
                scheduleElfRespawn(i, i % 2);
                continue;
            }
        }
        if (checkCollision(santaHitX, santaHitY, santaHitW, santaHitH,
                elves[i].x, elves[i].y, ELF_SIZE, ELF_SIZE)) {
            collectGift();
            scheduleElfRespawn(i, i % 2);
            continue;
        }
        updateElfMark(i);
        SPR_setPosition(elves[i].sprite, elves[i].x, elves[i].y);
    }

    for (u8 i = 0; i < NUM_ENEMIES; i++) {
        if (!enemies[i].active) continue;
        enemies[i].y += scrollStep;
        if ((frameCounter % ENEMY_LATERAL_DELAY) == 0) {
            if (enemies[i].x < santa.x) enemies[i].x += ENEMY_LATERAL_SPEED;
            else if (enemies[i].x > santa.x) enemies[i].x -= ENEMY_LATERAL_SPEED;
        }
        if (enemies[i].y > SCREEN_HEIGHT) {
            spawnEnemy(&enemies[i]);
        }
        if (checkCollision(
                santaHitX, santaHitY, santaHitW, santaHitH,
                enemies[i].x,
                enemies[i].y + (ENEMY_SIZE - ENEMY_HITBOX_HEIGHT),
                ENEMY_SIZE, ENEMY_HITBOX_HEIGHT)) {
            if (giftsCollected > 0) {
                giftsCollected--;
            }
            spawnEnemy(&enemies[i]);
        }
        SPR_setPosition(enemies[i].sprite, enemies[i].x, enemies[i].y);
    }

    reorderDepthByBottom();

    frameCounter++;
}

void minigamePickup_render(void) {
#if DEBUG_MODE
    renderDebug();
#endif

    SPR_update();
    SYS_doVBlankProcess();
}

u8 minigamePickup_isComplete(void) {
    return (giftsCollected >= TARGET_GIFTS);
}
