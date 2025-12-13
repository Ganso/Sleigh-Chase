/**
 * @file minigame_delivery.c
 * @brief Fase 2: Entrega - estructura base jugable con chimeneas y enemigos.
 */

#include "minigame_delivery.h"

#include <string.h>

#include "audio_manager.h"
#include "hud.h"
#include "resources_bg.h"
#include "resources_sfx.h"
#include "resources_sprites.h"
#include "snow_effect.h"

#define DELIVERY_TARGET 10
#define NUM_CHIMNEYS 8
#define NUM_ENEMIES 3
#define NUM_GIFT_DROPS 3
#define WORLD_WIDTH 512
#define WORLD_HEIGHT SCREEN_HEIGHT
#define CHIMNEY_SIZE 48
#define ENEMY_SIZE 32
#define DROP_COOLDOWN_FRAMES 18
#define CHIMNEY_RESET_FRAMES 90
#define RECOVERY_FRAMES 90
#define GIFT_DROP_SPEED 3

#define SANTA_WIDTH 80
#define SANTA_HEIGHT 128
#define SANTA_HITBOX_WIDTH 44
#define SANTA_HITBOX_HEIGHT 40
#define SANTA_START_Y (SCREEN_HEIGHT - SANTA_HEIGHT + 8)

#define DEPTH_BACKGROUND (SPR_MIN_DEPTH + 4)
#define DEPTH_SANTA SPR_MIN_DEPTH
#define DEPTH_EFFECTS (SPR_MIN_DEPTH + 2)
#define DEPTH_HUD (SPR_MIN_DEPTH + 10)

enum {
    CHIMNEY_INACTIVE = 0,
    CHIMNEY_ACTIVE = 1,
    CHIMNEY_COOLDOWN = 2,
};

typedef struct {
    Sprite* sprite;
    s16 x;
    s16 y;
    u8 state;
    u16 cooldown;
    u8 blink;
} Chimney;

typedef struct {
    Sprite* sprite;
    s16 x;
    s16 y;
    s8 vx;
    s8 vy;
    u8 active;
} Enemy;

typedef struct {
    Sprite* sprite;
    s16 x;
    s16 y;
    s16 vy;
    u8 active;
} GiftDrop;

typedef struct {
    Sprite* sprite;
    s16 x;
    s16 y;
    s8 vx;
    s8 vy;
} Santa;

static const GameInertia santaInertia = { 1, 1, 2, 5 };
static const Vect2D_s16 chimneyPositions[NUM_CHIMNEYS] = {
    { 30, 144 }, { 96, 120 }, { 160, 136 }, { 220, 116 },
    { 300, 132 }, { 360, 118 }, { 428, 140 }, { 482, 124 },
};

static Chimney chimneys[NUM_CHIMNEYS];
static Enemy enemies[NUM_ENEMIES];
static GiftDrop drops[NUM_GIFT_DROPS];
static Santa santa;

static Map* mapBackground;
static SnowEffect snowEffect;
static GameTimer gameTimer;

static Sprite* giftCounterTop;
static Sprite* giftCounterBottom;

static s16 cameraX;
static u16 frameCounter;
static u8 giftsRemaining;
static u8 deliveriesCompleted;
static u8 phaseCompleted;
static s8 dropCooldown;
static u16 recoveringFrames;
static u16 previousInput;

static void initBackground(void);
static void initSanta(void);
static void initChimneys(void);
static void initEnemies(void);
static void initGiftDrops(void);
static void initGiftCounterSprites(void);
static void updateCamera(void);
static void updateChimneys(void);
static void updateEnemies(void);
static void updateGiftDrops(void);
static void updateGiftCounter(void);
static void spawnGiftDrop(void);
static void handleGiftDrop(void);
static void checkEnemyCollision(void);
static void startRecovery(void);
static void updateRecovery(void);
static u8 isSantaOverChimney(const Chimney* chimney);
static u8 checkCollision(s16 x1, s16 y1, s16 w1, s16 h1, s16 x2, s16 y2, s16 w2, s16 h2);

/** @brief Configura recursos, HUD y estado inicial de la fase. */
void minigameDelivery_init(void) {
    VDP_setScreenWidth320();
    VDP_setScreenHeight224();

    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);

    frameCounter = 0;
    giftsRemaining = DELIVERY_TARGET;
    deliveriesCompleted = 0;
    phaseCompleted = FALSE;
    dropCooldown = 0;
    recoveringFrames = 0;
    previousInput = 0;
    cameraX = 0;

    hud_init();
    hud_reset_phase(2);

    initBackground();
    initChimneys();
    initEnemies();
    initGiftDrops();
    initGiftCounterSprites();
    initSanta();
    updateGiftCounter();

    gameCore_initTimer(&gameTimer, 120);

    XGM2_playPCM(snd_santa_hohoho, sizeof(snd_santa_hohoho), SOUND_PCM_CH_AUTO);
}

/** @brief Actualiza entrada, movimiento y lógica principal de la fase. */
void minigameDelivery_update(void) {
    frameCounter++;

    snowEffect_update(&snowEffect, frameCounter);

    u16 input = gameCore_readInput();

    if (recoveringFrames > 0) {
        updateRecovery();
    } else {
        s8 dirX = 0;
        s8 dirY = 0;
        if (input & BUTTON_LEFT) dirX = -1;
        else if (input & BUTTON_RIGHT) dirX = 1;
        if (input & BUTTON_UP) dirY = -1;
        else if (input & BUTTON_DOWN) dirY = 1;

        gameCore_applyInertiaMovement(&santa.x, &santa.y, &santa.vx, &santa.vy,
            dirX, dirY,
            0, WORLD_WIDTH - SANTA_WIDTH,
            0, WORLD_HEIGHT - SANTA_HEIGHT,
            frameCounter, &santaInertia);

        if (dropCooldown > 0) {
            dropCooldown--;
        }

        if ((input & BUTTON_A) && !(previousInput & BUTTON_A) && dropCooldown == 0) {
            handleGiftDrop();
        }
    }

    updateCamera();
    updateChimneys();
    updateEnemies();
    updateGiftDrops();

    SPR_setPosition(santa.sprite, santa.x - cameraX, santa.y);

    if (recoveringFrames == 0) {
        checkEnemyCollision();
    }

    s32 remainingFrames = gameCore_updateTimer(&gameTimer);
    u16 secondsRemaining = remainingFrames > 0 ? (remainingFrames / 60) : 0;
    hud_draw_delivery(deliveriesCompleted, DELIVERY_TARGET, secondsRemaining);
    updateGiftCounter();

    if (giftsRemaining == 0 || deliveriesCompleted >= DELIVERY_TARGET) {
        phaseCompleted = TRUE;
    }

    previousInput = input;
}

/** @brief Sincroniza sprites y espera a VBlank. */
void minigameDelivery_render(void) {
    SPR_update();
    SYS_doVBlankProcess();
}

/** @brief Indica si se alcanzó el objetivo de entregas. */
u8 minigameDelivery_isComplete(void) {
    return phaseCompleted;
}

static void initBackground(void) {
    gameCore_resetTileIndex();

    if (image_fondo_tejados_pal.data) {
        PAL_setPalette(PAL_COMMON, image_fondo_tejados_pal.data, CPU);
    }
    if (sprite_santa_car.palette) {
        PAL_setPalette(PAL_PLAYER, sprite_santa_car.palette->data, CPU);
    }
    if (sprite_duende_malo.palette) {
        PAL_setPalette(PAL_EFFECT, sprite_duende_malo.palette->data, CPU);
    }

    VDP_setBackgroundColor(0);

    VDP_loadTileSet(&image_fondo_tejados_tile, globalTileIndex, CPU);
    mapBackground = MAP_create(&image_fondo_tejados_map, BG_B,
        TILE_ATTR_FULL(PAL_COMMON, FALSE, FALSE, FALSE, globalTileIndex));
    globalTileIndex += image_fondo_tejados_tile.numTile;
    if (mapBackground != NULL) {
        MAP_scrollTo(mapBackground, 0, 0);
    }
    SYS_doVBlankProcess();

    snowEffect_init(&snowEffect, &globalTileIndex, 1, -2);
}

static void initSanta(void) {
    santa.x = (WORLD_WIDTH - SANTA_WIDTH) / 2;
    santa.y = SANTA_START_Y;
    santa.vx = 0;
    santa.vy = 0;

    santa.sprite = SPR_addSpriteSafe(&sprite_santa_car, santa.x, santa.y,
        TILE_ATTR(PAL_PLAYER, FALSE, FALSE, FALSE));
    SPR_setDepth(santa.sprite, DEPTH_SANTA);
    SPR_setAutoAnimation(santa.sprite, TRUE);
}

static void initChimneys(void) {
    memset(chimneys, 0, sizeof(chimneys));
    for (u8 i = 0; i < NUM_CHIMNEYS; i++) {
        chimneys[i].x = chimneyPositions[i].x;
        chimneys[i].y = chimneyPositions[i].y;
        chimneys[i].state = CHIMNEY_ACTIVE;
        chimneys[i].cooldown = 0;
        chimneys[i].blink = 0;
        chimneys[i].sprite = SPR_addSpriteSafe(&sprite_chimenea,
            chimneys[i].x, chimneys[i].y,
            TILE_ATTR(PAL_PLAYER, FALSE, FALSE, FALSE));
        if (chimneys[i].sprite) {
            SPR_setDepth(chimneys[i].sprite, DEPTH_BACKGROUND);
        }
    }
}

static void initEnemies(void) {
    memset(enemies, 0, sizeof(enemies));
    for (u8 i = 0; i < NUM_ENEMIES; i++) {
        enemies[i].active = TRUE;
        enemies[i].x = 40 + (i * 120);
        enemies[i].y = 60 + (i * 20);
        enemies[i].vx = (i % 2 == 0) ? 1 : -1;
        enemies[i].vy = 0;
        enemies[i].sprite = SPR_addSpriteSafe(&sprite_duende_malo, enemies[i].x, enemies[i].y,
            TILE_ATTR(PAL_EFFECT, FALSE, FALSE, FALSE));
        if (enemies[i].sprite) {
            SPR_setDepth(enemies[i].sprite, DEPTH_EFFECTS);
        }
    }
}

static void initGiftDrops(void) {
    memset(drops, 0, sizeof(drops));
    for (u8 i = 0; i < NUM_GIFT_DROPS; i++) {
        drops[i].sprite = SPR_addSpriteSafe(&sprite_regalo_entrega, 0, 0,
            TILE_ATTR(PAL_PLAYER, FALSE, FALSE, FALSE));
        drops[i].active = FALSE;
        drops[i].vy = GIFT_DROP_SPEED;
        if (drops[i].sprite) {
            SPR_setDepth(drops[i].sprite, DEPTH_EFFECTS);
            SPR_setVisibility(drops[i].sprite, HIDDEN);
        }
    }
}

static void initGiftCounterSprites(void) {
    const s16 baseX = SCREEN_WIDTH - 140;
    const s16 baseY = SCREEN_HEIGHT - 40;

    giftCounterTop = SPR_addSpriteSafe(&sprite_icono_regalo, baseX, baseY - 16,
        TILE_ATTR(PAL_EFFECT, FALSE, FALSE, FALSE));
    if (giftCounterTop) {
        SPR_setAutoAnimation(giftCounterTop, FALSE);
        SPR_setDepth(giftCounterTop, DEPTH_HUD + 1);
    }

    giftCounterBottom = SPR_addSpriteSafe(&sprite_icono_regalo, baseX + 12, baseY,
        TILE_ATTR(PAL_EFFECT, FALSE, FALSE, FALSE));
    if (giftCounterBottom) {
        SPR_setAutoAnimation(giftCounterBottom, FALSE);
        SPR_setDepth(giftCounterBottom, DEPTH_HUD);
    }
}

static void updateCamera(void) {
    s16 desiredX = santa.x - (SCREEN_WIDTH / 2) + (SANTA_WIDTH / 2);
    if (desiredX < 0) desiredX = 0;
    if (desiredX > (WORLD_WIDTH - SCREEN_WIDTH)) desiredX = WORLD_WIDTH - SCREEN_WIDTH;
    cameraX = desiredX;

    if (mapBackground != NULL) {
        MAP_scrollTo(mapBackground, cameraX, 0);
    }
}

static void updateChimneys(void) {
    for (u8 i = 0; i < NUM_CHIMNEYS; i++) {
        Chimney* chimney = &chimneys[i];
        if (chimney->state == CHIMNEY_COOLDOWN) {
            if (chimney->cooldown > 0) {
                chimney->cooldown--;
                if ((chimney->cooldown % 6) == 0) {
                    chimney->blink = !chimney->blink;
                }
            } else {
                chimney->state = CHIMNEY_ACTIVE;
                chimney->blink = 0;
            }
        }

        s16 screenX = chimney->x - cameraX;
        u8 visible = (screenX > -CHIMNEY_SIZE) && (screenX < SCREEN_WIDTH);
        if (chimney->sprite) {
            SPR_setPosition(chimney->sprite, screenX, chimney->y);
            if (chimney->state == CHIMNEY_COOLDOWN && chimney->blink) {
                SPR_setVisibility(chimney->sprite, HIDDEN);
            } else {
                SPR_setVisibility(chimney->sprite, visible ? VISIBLE : HIDDEN);
            }
        }
    }
}

static void updateEnemies(void) {
    for (u8 i = 0; i < NUM_ENEMIES; i++) {
        Enemy* enemy = &enemies[i];
        if (!enemy->active || enemy->sprite == NULL) continue;

        enemy->x += enemy->vx;
        if (enemy->x < 0) {
            enemy->x = 0;
            enemy->vx = 1;
        } else if (enemy->x > (WORLD_WIDTH - ENEMY_SIZE)) {
            enemy->x = WORLD_WIDTH - ENEMY_SIZE;
            enemy->vx = -1;
        }

        enemy->y += (frameCounter % 60 == 0) ? 1 : 0;
        if (enemy->y > (SCREEN_HEIGHT - ENEMY_SIZE - 40)) {
            enemy->y = 40;
        }

        s16 screenX = enemy->x - cameraX;
        SPR_setPosition(enemy->sprite, screenX, enemy->y);
        SPR_setVisibility(enemy->sprite, (screenX > -ENEMY_SIZE && screenX < SCREEN_WIDTH) ? VISIBLE : HIDDEN);
    }
}

static void updateGiftDrops(void) {
    for (u8 i = 0; i < NUM_GIFT_DROPS; i++) {
        GiftDrop* drop = &drops[i];
        if (!drop->active || drop->sprite == NULL) continue;

        drop->y += drop->vy;
        if (drop->y > SCREEN_HEIGHT) {
            drop->active = FALSE;
            SPR_setVisibility(drop->sprite, HIDDEN);
            continue;
        }

        SPR_setPosition(drop->sprite, drop->x - cameraX, drop->y);
        SPR_setVisibility(drop->sprite, VISIBLE);
    }
}

static void updateGiftCounter(void) {
    const s16 baseX = SCREEN_WIDTH - 140;
    const s16 baseY = SCREEN_HEIGHT - 40;

    u16 capped = giftsRemaining;
    if (capped > DELIVERY_TARGET) capped = DELIVERY_TARGET;

    u16 topFrame = (capped > 5) ? 5 : capped;
    u16 bottomFrame = (capped > 5) ? (capped - 5) : 0;

    if (giftCounterTop) {
        SPR_setAnim(giftCounterTop, 0);
        SPR_setFrame(giftCounterTop, topFrame);
        SPR_setPosition(giftCounterTop, baseX, baseY - 16);
        SPR_setDepth(giftCounterTop, DEPTH_HUD + 1);
        SPR_setVisibility(giftCounterTop, VISIBLE);
    }
    if (giftCounterBottom) {
        SPR_setAnim(giftCounterBottom, 0);
        SPR_setFrame(giftCounterBottom, bottomFrame);
        SPR_setPosition(giftCounterBottom, baseX + 12, baseY);
        SPR_setDepth(giftCounterBottom, DEPTH_HUD);
        SPR_setVisibility(giftCounterBottom, VISIBLE);
    }
}

static void spawnGiftDrop(void) {
    for (u8 i = 0; i < NUM_GIFT_DROPS; i++) {
        GiftDrop* drop = &drops[i];
        if (drop->active || drop->sprite == NULL) continue;

        drop->active = TRUE;
        drop->x = santa.x + (SANTA_WIDTH / 2) - 12;
        drop->y = santa.y + (SANTA_HEIGHT / 2);
        SPR_setPosition(drop->sprite, drop->x - cameraX, drop->y);
        SPR_setVisibility(drop->sprite, VISIBLE);
        break;
    }
}

static void handleGiftDrop(void) {
    if (giftsRemaining == 0) return;

    for (u8 i = 0; i < NUM_CHIMNEYS; i++) {
        Chimney* chimney = &chimneys[i];
        if (chimney->state != CHIMNEY_ACTIVE) continue;

        if (isSantaOverChimney(chimney)) {
            chimney->state = CHIMNEY_COOLDOWN;
            chimney->cooldown = CHIMNEY_RESET_FRAMES;
            chimney->blink = 0;
            deliveriesCompleted++;
            giftsRemaining--;
            dropCooldown = DROP_COOLDOWN_FRAMES;
            spawnGiftDrop();
            break;
        }
    }
}

static void checkEnemyCollision(void) {
    s16 santaHitX = santa.x + ((SANTA_WIDTH - SANTA_HITBOX_WIDTH) / 2);
    s16 santaHitY = santa.y + (SANTA_HEIGHT - SANTA_HITBOX_HEIGHT);

    for (u8 i = 0; i < NUM_ENEMIES; i++) {
        Enemy* enemy = &enemies[i];
        if (!enemy->active) continue;

        if (checkCollision(santaHitX, santaHitY, SANTA_HITBOX_WIDTH, SANTA_HITBOX_HEIGHT,
                enemy->x, enemy->y, ENEMY_SIZE, ENEMY_SIZE)) {
            startRecovery();
            break;
        }
    }
}

static void startRecovery(void) {
    recoveringFrames = RECOVERY_FRAMES;
    dropCooldown = DROP_COOLDOWN_FRAMES;
    santa.x = (WORLD_WIDTH - SANTA_WIDTH) / 2;
    santa.y = SANTA_START_Y;
    santa.vx = 0;
    santa.vy = 0;
    SPR_setPosition(santa.sprite, santa.x - cameraX, santa.y);
}

static void updateRecovery(void) {
    if (recoveringFrames == 0) return;

    recoveringFrames--;
    if (santa.sprite) {
        if ((recoveringFrames % 8) == 0) {
            SPR_setVisibility(santa.sprite, HIDDEN);
        } else if ((recoveringFrames % 4) == 0) {
            SPR_setVisibility(santa.sprite, VISIBLE);
        }
    }

    if (recoveringFrames == 0 && santa.sprite) {
        SPR_setVisibility(santa.sprite, VISIBLE);
    }
}

static u8 isSantaOverChimney(const Chimney* chimney) {
    if (chimney == NULL) return FALSE;

    s16 santaHitX = santa.x + ((SANTA_WIDTH - SANTA_HITBOX_WIDTH) / 2);
    s16 santaHitY = santa.y + (SANTA_HEIGHT - SANTA_HITBOX_HEIGHT);

    return checkCollision(santaHitX, santaHitY, SANTA_HITBOX_WIDTH, SANTA_HITBOX_HEIGHT,
        chimney->x, chimney->y, CHIMNEY_SIZE, CHIMNEY_SIZE);
}

static u8 checkCollision(s16 x1, s16 y1, s16 w1, s16 h1, s16 x2, s16 y2, s16 w2, s16 h2) {
    if (x1 + w1 < x2) return FALSE;
    if (x2 + w2 < x1) return FALSE;
    if (y1 + h1 < y2) return FALSE;
    if (y2 + h2 < y1) return FALSE;
    return TRUE;
}

