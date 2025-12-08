/**
 * ═════════════════════════════════════════════════════════════════════════════
 * ARCHIVO: src/minigame_pickup.c
 *
 * Fase 1: Recogida - Paseo cenital con pista de nieve y scroll vertical
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
#define TREE_HITBOX_SIZE (TREE_SIZE / 2)
#define ENEMY_SIZE 32
#define ENEMY_HITBOX_SIZE (ENEMY_SIZE / 2)
#define ELF_SIZE 32
#define SANTA_WIDTH 80
#define SANTA_HEIGHT 128
#define SANTA_HITBOX_PADDING 20   /* píxeles que no colisionan a cada lado */
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

typedef struct {
    s8 accel;
    s8 friction;
    s8 maxVelocity;
} InertiaConfig;

static Santa santa;
static SimpleActor trees[NUM_TREES];
static SimpleActor elves[NUM_ELVES];
static SimpleActor enemies[NUM_ENEMIES];
static Map *mapTrack;
static s16 trackHeightPx;
static s16 trackOffsetY;
static s16 trackMaxScroll;
static u16 giftsCollected;
static u16 giftsCharge;
static u16 frameCounter;

static s16 leftLimit;
static s16 rightLimit;
static s16 playableWidth;
static s16 santaMinY;
static s16 santaMaxY;
static InertiaConfig santaInertia;
static SnowEffect snowEffect;

int kprintf(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

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

static void spawnElf(SimpleActor *elf, u8 side) {
    elf->x = (side == 0) ? (leftLimit - ELF_SIZE - 3) : (rightLimit + 3);
    elf->y = -((random() % (SCREEN_HEIGHT - 60)) + 60);
    elf->active = TRUE;

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

static void resetSpecialIfReady(void) {
    if (giftsCharge >= GIFTS_FOR_SPECIAL) {
        santa.specialReady = TRUE;
        /* Animación alternativa desactivada para evitar errores si no existe */
        SPR_setAnim(santa.sprite, 0);
    }
}

static void clampTrackOffset(void) {
    if (trackOffsetY < 0) {
        KLog("[TRACK] Offset negativo detectado, se fuerza a 0");
        trackOffsetY = 0;
    }
    if (trackOffsetY > trackMaxScroll) {
        kprintf("[TRACK] Offset %d supera maximo %d, se recorta", trackOffsetY, trackMaxScroll);
        trackOffsetY = trackMaxScroll;
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

static void applyInertiaAxis(s16 *position, s8 *velocity, s16 minLimit, s16 maxLimit,
    s8 inputDirection, const InertiaConfig *config) {
    if (inputDirection < 0) {
        *velocity = (*velocity > -config->maxVelocity) ?
            *velocity - config->accel : -config->maxVelocity;
    } else if (inputDirection > 0) {
        *velocity = (*velocity < config->maxVelocity) ?
            *velocity + config->accel : config->maxVelocity;
    } else {
        if (*velocity > 0) {
            *velocity -= config->friction;
            if (*velocity < 0) *velocity = 0;
        } else if (*velocity < 0) {
            *velocity += config->friction;
            if (*velocity > 0) *velocity = 0;
        }
    }

    *position += *velocity;
    if (*position < minLimit) *position = minLimit;
    if (*position > maxLimit) *position = maxLimit;
}

static void applyInertiaMovement(s16 *x, s16 *y, s8 *vx, s8 *vy,
    s8 inputX, s8 inputY, s16 minX, s16 maxX, s16 minY, s16 maxY,
    const InertiaConfig *inertia) {
    applyInertiaAxis(x, vx, minX, maxX, inputX, inertia);
    applyInertiaAxis(y, vy, minY, maxY, inputY, inertia);
}

#if DEBUG_MODE
static void renderDebug(void) {
    char buffer[48];

    //sprintf(buffer, "DBG frame:%lu off:%d/%d", (u32)frameCounter, trackOffsetY, trackMaxScroll);
    KLog(buffer);
}
#endif

void minigamePickup_init(void) {
    VDP_setScreenWidth320();
    VDP_setScreenHeight224();
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);

    gameCore_resetTileIndex();
    trackOffsetY = 0;
    trackHeightPx = 0;
    trackMaxScroll = 0;
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
    santaInertia.maxVelocity = 3; /* velocidad máxima más baja para un movimiento más lento y con inercia */

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

    /* Usar el alto real del mapa para evitar desbordar VRAM durante el scroll */
    if (mapTrack != NULL && mapTrack->h > 0) {
        trackHeightPx = mapTrack->h << 3; /* 8 px por tile */
    } else {
        trackHeightPx = TRACK_HEIGHT_PX; /* Fallback al tamaño previsto 320x640 */
    }
    trackMaxScroll = trackHeightPx - SCREEN_HEIGHT;
    if (trackMaxScroll < 0) trackMaxScroll = 0;
    trackOffsetY = trackMaxScroll;
    kprintf("[TRACK] alto_px:%d max:%d", trackHeightPx, trackMaxScroll);
    if (mapTrack == NULL) {
        trackHeightPx = 0;
        trackMaxScroll = 0;
        trackOffsetY = 0;
    } else {
        MAP_scrollTo(mapTrack, 0, trackOffsetY);
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
        spawnElf(&elves[i], i % 2);
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

    applyInertiaMovement(&santa.x, &santa.y, &santa.vx, &santa.vy,
        inputDirX, inputDirY,
        leftLimit, leftLimit + playableWidth,
        santaMinY, santaMaxY,
        &santaInertia);
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
        if (trackOffsetY < 0) {
            trackOffsetY = trackMaxScroll;
        }
        clampTrackOffset();
        if (mapTrack != NULL) {
            MAP_scrollTo(mapTrack, 0, trackOffsetY);
            VDP_setVerticalScroll(BG_B, trackOffsetY);
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
                trees[i].x + (TREE_SIZE - TREE_HITBOX_SIZE) / 2,
                trees[i].y + (TREE_SIZE - TREE_HITBOX_SIZE) / 2,
                TREE_HITBOX_SIZE, TREE_HITBOX_SIZE)) {
            collectGift();
            spawnTree(&trees[i]);
        }
        SPR_setPosition(trees[i].sprite, trees[i].x, trees[i].y);
    }

    for (u8 i = 0; i < NUM_ELVES; i++) {
        if (!elves[i].active) continue;
        if (scrollStep) {
            elves[i].y += scrollStep;
            if (elves[i].y > SCREEN_HEIGHT) {
                spawnElf(&elves[i], i % 2);
            }
        }
        if (checkCollision(santaHitX, santaHitY, santaHitW, santaHitH,
                elves[i].x, elves[i].y, ELF_SIZE, ELF_SIZE)) {
            collectGift();
            spawnElf(&elves[i], i % 2);
        }
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
                enemies[i].x + (ENEMY_SIZE - ENEMY_HITBOX_SIZE) / 2,
                enemies[i].y + (ENEMY_SIZE - ENEMY_HITBOX_SIZE) / 2,
                ENEMY_HITBOX_SIZE, ENEMY_HITBOX_SIZE)) {
            if (giftsCollected > 0) {
                giftsCollected--;
            }
            spawnEnemy(&enemies[i]);
        }
        SPR_setPosition(enemies[i].sprite, enemies[i].x, enemies[i].y);
    }

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
