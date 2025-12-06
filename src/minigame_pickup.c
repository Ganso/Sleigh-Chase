/**
 * ═════════════════════════════════════════════════════════════════════════════
 * ARCHIVO: src/minigame_pickup.c
 *
 * Fase 1: Recogida - Paseo cenital con pista de nieve y scroll vertical
 * ═════════════════════════════════════════════════════════════════════════════
 */

#include "minigame_pickup.h"
#include "resources.h"

#define NUM_TREES 4
#define NUM_ELVES 2
#define NUM_ENEMIES 4
#define GIFTS_FOR_SPECIAL 3
#define TARGET_GIFTS 15
#define SCROLL_SPEED 1
#define FORBIDDEN_PERCENT 20
#define TRACK_HEIGHT_TILES 56   // 320x448px (dos pantallas)

#define ENEMY_LATERAL_DELAY 30
#define ENEMY_LATERAL_SPEED 1

#define TREE_SIZE 64
#define ENEMY_SIZE 32
#define ELF_SIZE 32
#define SANTA_WIDTH 80
#define SANTA_HEIGHT 128

typedef struct {
    Sprite* sprite;
    s16 x, y;
    u8 active;
} SimpleActor;

typedef struct {
    Sprite* sprite;
    s16 x, y;
    s8 vx;
    u8 specialReady;
} Santa;

static Santa santa;
static SimpleActor trees[NUM_TREES];
static SimpleActor elves[NUM_ELVES];
static SimpleActor enemies[NUM_ENEMIES];
static Map *mapTrack;
static s16 trackOffsetY;
static u32 tileIndex;
static u16 giftsCollected;
static u16 giftsCharge;
static u16 frameCounter;

static s16 leftLimit;
static s16 rightLimit;
static s16 playableWidth;

static void placeActor(SimpleActor *actor, s16 minX, s16 maxX, s16 minY, s16 maxY) {
    actor->x = minX + (random() % (maxX - minX));
    actor->y = -((random() % (maxY - minY)) + minY);
    actor->active = TRUE;
}

static void spawnTree(SimpleActor *tree) {
    placeActor(tree, leftLimit, rightLimit - TREE_SIZE, 40, SCREEN_HEIGHT);
    if (tree->sprite == NULL) {
        tree->sprite = SPR_addSpriteSafe(&sprite_arbol_pista, tree->x, tree->y,
            TILE_ATTR(PAL_COMMON, FALSE, FALSE, FALSE));
    }
    SPR_setPosition(tree->sprite, tree->x, tree->y);
}

static void spawnElf(SimpleActor *elf, u8 side) {
    s16 areaWidth = (SCREEN_WIDTH * FORBIDDEN_PERCENT) / 100;
    s16 baseX = side == 0 ? 8 : SCREEN_WIDTH - areaWidth + 8;
    placeActor(elf, baseX, baseX + areaWidth - ELF_SIZE, 60, SCREEN_HEIGHT);
    if (elf->sprite == NULL) {
        elf->sprite = SPR_addSpriteSafe(&sprite_elfo_lateral, elf->x, elf->y,
            TILE_ATTR(PAL_PLAYER, FALSE, FALSE, FALSE));
    }
    SPR_setPosition(elf->sprite, elf->x, elf->y);
}

static void spawnEnemy(SimpleActor *enemy) {
    placeActor(enemy, leftLimit, rightLimit - ENEMY_SIZE, 30, SCREEN_HEIGHT);
    if (enemy->sprite == NULL) {
        enemy->sprite = SPR_addSpriteSafe(&sprite_duende_malo, enemy->x, enemy->y,
            TILE_ATTR(PAL_EFFECT, FALSE, FALSE, FALSE));
    }
    SPR_setPosition(enemy->sprite, enemy->x, enemy->y);
}

static void resetSpecialIfReady(void) {
    if (giftsCharge >= GIFTS_FOR_SPECIAL) {
        santa.specialReady = TRUE;
        SPR_setAnim(santa.sprite, 1);
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

void minigamePickup_init(void) {
    VDP_setScreenWidth320();
    VDP_setScreenHeight224();
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);

    tileIndex = TILE_USER_INDEX;
    trackOffsetY = 0;
    giftsCollected = 0;
    giftsCharge = 0;
    frameCounter = 0;

    leftLimit = (SCREEN_WIDTH * FORBIDDEN_PERCENT) / 100;
    rightLimit = SCREEN_WIDTH - leftLimit;
    playableWidth = rightLimit - leftLimit - SANTA_WIDTH;

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

    VDP_loadTileSet(&image_pista_polo_tile, tileIndex, CPU);
    mapTrack = MAP_create(&image_pista_polo_map, BG_B,
        TILE_ATTR_FULL(PAL_COMMON, FALSE, FALSE, FALSE, tileIndex));
    tileIndex += image_pista_polo_tile.numTile;
    MAP_scrollTo(mapTrack, 0, 0);

    santa.x = leftLimit + playableWidth / 2;
    santa.y = SCREEN_HEIGHT - (SANTA_HEIGHT / 2);
    santa.vx = 0;
    santa.specialReady = FALSE;
    santa.sprite = SPR_addSpriteSafe(&sprite_santa_car, santa.x, santa.y,
        TILE_ATTR(PAL_PLAYER, TRUE, FALSE, FALSE));
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

    if (input & BUTTON_LEFT) {
        santa.vx = -2;
    } else if (input & BUTTON_RIGHT) {
        santa.vx = 2;
    } else {
        santa.vx = 0;
    }

    if (santa.specialReady && (input & BUTTON_B)) {
        santa.specialReady = FALSE;
        giftsCharge = 0;
        SPR_setAnim(santa.sprite, 0);
        clearEnemies();
    }

    santa.x += santa.vx;
    if (santa.x < leftLimit) santa.x = leftLimit;
    if (santa.x > leftLimit + playableWidth) santa.x = leftLimit + playableWidth;
    SPR_setPosition(santa.sprite, santa.x, santa.y);

    trackOffsetY += SCROLL_SPEED;
    s16 maxScroll = (TRACK_HEIGHT_TILES * 8) - SCREEN_HEIGHT;
    if (trackOffsetY > maxScroll) {
        trackOffsetY = 0;
    }
    MAP_scrollTo(mapTrack, 0, trackOffsetY);

    for (u8 i = 0; i < NUM_TREES; i++) {
        if (!trees[i].active) continue;
        trees[i].y += SCROLL_SPEED;
        if (trees[i].y > SCREEN_HEIGHT) {
            spawnTree(&trees[i]);
        }
        if (checkCollision(santa.x, santa.y, SANTA_WIDTH, SANTA_HEIGHT,
                trees[i].x, trees[i].y, TREE_SIZE, TREE_SIZE)) {
            collectGift();
            spawnTree(&trees[i]);
        }
        SPR_setPosition(trees[i].sprite, trees[i].x, trees[i].y);
    }

    for (u8 i = 0; i < NUM_ELVES; i++) {
        elves[i].y += SCROLL_SPEED;
        if (elves[i].y > SCREEN_HEIGHT) {
            spawnElf(&elves[i], i % 2);
        }
        if (checkCollision(santa.x, santa.y, SANTA_WIDTH, SANTA_HEIGHT,
                elves[i].x, elves[i].y, ELF_SIZE, ELF_SIZE)) {
            collectGift();
            spawnElf(&elves[i], i % 2);
        }
        SPR_setPosition(elves[i].sprite, elves[i].x, elves[i].y);
    }

    for (u8 i = 0; i < NUM_ENEMIES; i++) {
        enemies[i].y += SCROLL_SPEED;
        if ((frameCounter % ENEMY_LATERAL_DELAY) == 0) {
            if (enemies[i].x < santa.x) enemies[i].x += ENEMY_LATERAL_SPEED;
            else if (enemies[i].x > santa.x) enemies[i].x -= ENEMY_LATERAL_SPEED;
        }
        if (enemies[i].y > SCREEN_HEIGHT) {
            spawnEnemy(&enemies[i]);
        }
        if (checkCollision(santa.x, santa.y, SANTA_WIDTH, SANTA_HEIGHT,
                enemies[i].x, enemies[i].y, ENEMY_SIZE, ENEMY_SIZE)) {
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
    char buffer[32];
    sprintf(buffer, "Regalos: %u/%u", giftsCollected, TARGET_GIFTS);
    VDP_drawText(buffer, 4, 2);

    sprintf(buffer, "Especial: %u/%u", giftsCharge, GIFTS_FOR_SPECIAL);
    VDP_drawText(buffer, 4, 4);

    SPR_update();
    SYS_doVBlankProcess();
}

u8 minigamePickup_isComplete(void) {
    return (giftsCollected >= TARGET_GIFTS);
}
