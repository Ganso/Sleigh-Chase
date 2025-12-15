/**
 * @file minigame_delivery.c
 * @brief Fase 2: Entrega - estructura base jugable con chimeneas y enemigos.
 *
 * Recursos y paletas empleados en la fase:
 * - Fondos y tiles de tejados definidos en `resources_bg.h` (mapa y tileset del
 *   tejado). La paleta asociada al fondo se carga desde ese mismo fichero y se
 *   aplica al `mapBackground`.
 * - Sprites de Santa, enemigos y regalos procedentes de `resources_sprites.h`.
 *   Cada sprite usa la paleta incluida en dicho fichero; se hace referencia a
 *   ella al crear los sprites en `initSanta`, `initEnemies` y `initGiftDrops`.
 * - Efectos de sonido de `resources_sfx.h`, compartiendo la paleta de sprites
 *   activa porque son muestras PCM sin influencia gráfica.
 * - Música de fase gestionada por `audio_manager.h`, que reutiliza
 *   `resources_music.h` como origen de los tracks.
 */

#include "minigame_delivery.h"
#include "audio_manager.h"
#include "resources_bg.h"
#include "resources_sfx.h"
#include "resources_sprites.h"
#include "snow_effect.h"

#define DELIVERY_TARGET 10              /* Regalos totales a entregar en la fase. */
#define MAX_VISIBLE_CHIMNEYS 4          /* Máximo de chimeneas simultáneas en pantalla. */
#define NUM_CHIMNEYS MAX_VISIBLE_CHIMNEYS /* Pool de chimeneas reutilizado. */
#define NUM_ENEMIES 2                   /* Enemigos concurrentes en el tejado. */
#define NUM_GIFT_DROPS 3                /* Regalos simultáneos en caída. */
#define WORLD_WIDTH SCREEN_WIDTH        /* Ancho jugable fijado a la pantalla. */
#define WORLD_HEIGHT 512                /* Altura total del bucle vertical. */
#define SCROLL_LOOP_PX WORLD_HEIGHT     /* Tamaño del loop de scroll en píxeles. */
#define SCROLL_SPEED_PER_FRAME FIX16(2) /* Velocidad de avance vertical. */
#define CHIMNEY_SIZE 32                 /* Tamaño (ancho/alto) de cada chimenea. */
#define CHIMNEY_MARGIN_X 4              /* Margen lateral respecto al borde. */
#define CHIMNEY_X_LEFT CHIMNEY_MARGIN_X /* Posición X para chimeneas de la izquierda. */
#define CHIMNEY_X_RIGHT (SCREEN_WIDTH - CHIMNEY_SIZE - CHIMNEY_MARGIN_X) /* X derecha. */
#define CHIMNEY_PROHIBITED_PERCENT 60   /* Probabilidad % de chimenea prohibida. */
#define HOUSE_HEIGHT 128                /* Altura de cada casa en el fondo. */
#define CHIMNEY_ROW_OFFSET 64           /* Offset vertical para centrar en las casas. */
#define ENEMY_WIDTH 48
#define ENEMY_HEIGHT 48
#define ENEMY_HORIZONTAL_SPEED 1
#define ENEMY_VERTICAL_SPEED 1
#define DROP_COOLDOWN_FRAMES 18
#define CHIMNEY_RESET_FRAMES 90
#define RECOVERY_FRAMES 90
#define GIFT_DROP_SPEED 3

#define SANTA_WIDTH 80
#define SANTA_HEIGHT 128
#define SANTA_HITBOX_WIDTH 44
#define SANTA_HITBOX_HEIGHT 40
#define SANTA_START_Y (SCREEN_HEIGHT - SANTA_HEIGHT + 8)

#define DEPTH_HUD SPR_MIN_DEPTH
#define DEPTH_SANTA (SPR_MIN_DEPTH + 24)
#define DEPTH_EFFECTS (DEPTH_SANTA + 8)
#define DEPTH_BACKGROUND (DEPTH_EFFECTS + 8)

enum {
    CHIMNEY_INACTIVE = 0,
    CHIMNEY_ACTIVE = 1,
    CHIMNEY_COOLDOWN = 2,
};

typedef struct {
    Sprite* sprite;
    Sprite* blockedSprite;
    s16 x;
    s16 y;
    u8 state;
    u16 cooldown;
    u8 blink;
    u8 prohibited;
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

static const GameInertia santaInertia = { 1, 1, 2, 5 }; /**< Configura aceleración, fricción y velocidad máxima del trineo. */
static Chimney chimneys[NUM_CHIMNEYS]; /**< Chimeneas activas en el tejado. */
static Enemy enemies[NUM_ENEMIES]; /**< Enemigos que roban regalos. */
static GiftDrop drops[NUM_GIFT_DROPS]; /**< Regalos en caída hacia chimeneas. */
static Santa santa; /**< Control del sprite principal. */

static Map* mapBackground; /**< Mapa del plano B para el tejado. */
static s16 backgroundOffsetY; /**< Offset vertical del scroll de fondo. */
static fix16 scrollAccumulator; /**< Acumulador de scroll fraccional. */
static fix16 scrollSpeedPerFrame; /**< Velocidad de scroll por frame. */
static SnowEffect snowEffect; /**< Efecto de nieve compartido. */
static GameTimer gameTimer; /**< Temporizador de la fase para derrota. */

static Sprite* giftCounterTop; /**< Contador gráfico fila superior. */
static Sprite* giftCounterBottom; /**< Contador gráfico fila inferior. */

static s16 nextChimneySpawnY; /**< Cursor vertical para nuevas chimeneas. */
static u16 frameCounter; /**< Contador global de frames. */
static u8 giftsRemaining; /**< Regalos que faltan por entregar. */
static u8 deliveriesCompleted; /**< Chimeneas servidas con éxito. */
static u8 phaseCompleted; /**< Marca de finalización de fase. */
static s8 dropCooldown; /**< Enfriamiento entre lanzamientos de regalo. */
static u16 recoveringFrames; /**< Ventana de invulnerabilidad tras daño. */
static u16 previousInput; /**< Entrada anterior para filtrar transiciones. */

static void initBackground(void);
static void initSanta(void);
static void initChimneys(void);
static void initEnemies(void);
static void initGiftDrops(void);
static void initGiftCounterSprites(void);
static s16 advanceVerticalScroll(void);
static void applyBackgroundScroll(void);
static void updateChimneys(s16 scrollStep);
static void updateEnemies(s16 scrollStep);
static void updateGiftDrops(s16 scrollStep);
static void respawnEnemyFromTop(Enemy* enemy, u8 offsetIndex);
static void resetChimneySpawnCursor(void);
static s16 takeNextChimneySpawnY(void);
static s16 pickChimneyX(void);
static u8 rollChimneyProhibited(void);
static void updateGiftCounter(void);
static void spawnGiftDrop(void);
static void handleGiftDrop(void);
static void checkEnemyCollision(void);
static void startRecovery(void);
static void updateRecovery(void);
static u8 isSantaOverChimney(const Chimney* chimney);

/** @brief Configura recursos, estado inicial de la fase. */
void minigameDelivery_init(void) {
    VDP_setScreenWidth320();
    VDP_setScreenHeight224();

    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);
    VDP_setPlaneSize(64, 64, TRUE);

    frameCounter = 0;
    giftsRemaining = DELIVERY_TARGET;
    deliveriesCompleted = 0;
    phaseCompleted = FALSE;
    dropCooldown = 0;
    recoveringFrames = 0;
    previousInput = 0;
    backgroundOffsetY = SCROLL_LOOP_PX;
    backgroundOffsetY %= SCROLL_LOOP_PX;
    scrollAccumulator = FIX16(0);
    scrollSpeedPerFrame = SCROLL_SPEED_PER_FRAME;

    
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
            0, SCREEN_HEIGHT - SANTA_HEIGHT,
            frameCounter, &santaInertia);

        if (dropCooldown > 0) {
            dropCooldown--;
        }

        if ((input & BUTTON_A) && !(previousInput & BUTTON_A) && dropCooldown == 0) {
            handleGiftDrop();
        }
    }

    s16 scrollStep = advanceVerticalScroll();
    applyBackgroundScroll();
    updateChimneys(scrollStep);
    updateEnemies(scrollStep);
    updateGiftDrops(scrollStep);

    SPR_setPosition(santa.sprite, santa.x, santa.y);

    if (recoveringFrames == 0) {
        checkEnemyCollision();
    }

    s32 remainingFrames = gameCore_updateTimer(&gameTimer);
    u16 secondsRemaining = remainingFrames > 0 ? (remainingFrames / 60) : 0;
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

    scrollAccumulator = FIX16(0);
    scrollSpeedPerFrame = SCROLL_SPEED_PER_FRAME;

    if (image_fondo_tejados_pal.data) {
        PAL_setPalette(PAL_COMMON, image_fondo_tejados_pal.data, CPU);
    }
    if (sprite_santa_car.palette) {
        PAL_setPalette(PAL_PLAYER, sprite_santa_car.palette->data, CPU);
    }
    if (sprite_duende_malo_volador.palette) {
        PAL_setPalette(PAL_EFFECT, sprite_duende_malo_volador.palette->data, CPU);
    }

    VDP_setBackgroundColor(0);

    VDP_loadTileSet(&image_fondo_tejados_tile, globalTileIndex, CPU);
    mapBackground = MAP_create(&image_fondo_tejados_map, BG_B,
        TILE_ATTR_FULL(PAL_COMMON, FALSE, FALSE, FALSE, globalTileIndex));
    globalTileIndex += image_fondo_tejados_tile.numTile;
    if (mapBackground != NULL) {
        MAP_scrollTo(mapBackground, 0, backgroundOffsetY);
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
    resetChimneySpawnCursor();
    for (u8 i = 0; i < NUM_CHIMNEYS; i++) {
        chimneys[i].state = CHIMNEY_ACTIVE;
        chimneys[i].cooldown = 0;
        chimneys[i].blink = 0;
        chimneys[i].x = pickChimneyX();
        chimneys[i].y = takeNextChimneySpawnY();
        chimneys[i].prohibited = rollChimneyProhibited();
        chimneys[i].state = chimneys[i].prohibited ? CHIMNEY_INACTIVE : CHIMNEY_ACTIVE;

        chimneys[i].sprite = SPR_addSpriteSafe(&sprite_chimenea,
            chimneys[i].x, chimneys[i].y,
            TILE_ATTR(PAL_PLAYER, FALSE, FALSE, FALSE));
        chimneys[i].blockedSprite = SPR_addSpriteSafe(&sprite_chimenea_prohibida,
            chimneys[i].x, chimneys[i].y,
            TILE_ATTR(PAL_PLAYER, FALSE, FALSE, FALSE));
        const u8 visible = (chimneys[i].y + CHIMNEY_SIZE > 0) && (chimneys[i].y < SCREEN_HEIGHT);
        if (chimneys[i].sprite) {
            SPR_setDepth(chimneys[i].sprite, DEPTH_BACKGROUND);
            SPR_setVisibility(chimneys[i].sprite,
                (!chimneys[i].prohibited && visible) ? VISIBLE : HIDDEN);
        }
        if (chimneys[i].blockedSprite) {
            SPR_setDepth(chimneys[i].blockedSprite, DEPTH_BACKGROUND);
            SPR_setVisibility(chimneys[i].blockedSprite,
                (chimneys[i].prohibited && visible) ? VISIBLE : HIDDEN);
        }
    }
}

static void initEnemies(void) {
    memset(enemies, 0, sizeof(enemies));
    for (u8 i = 0; i < NUM_ENEMIES; i++) {
        enemies[i].active = TRUE;
        enemies[i].vx = 0;
        enemies[i].vy = ENEMY_VERTICAL_SPEED;
        enemies[i].sprite = SPR_addSpriteSafe(&sprite_duende_malo_volador, 0, 0,
            TILE_ATTR(PAL_EFFECT, FALSE, FALSE, FALSE));
        if (enemies[i].sprite) {
            SPR_setDepth(enemies[i].sprite, DEPTH_EFFECTS);
            SPR_setAutoAnimation(enemies[i].sprite, TRUE);
        } else {
            enemies[i].active = FALSE;
            continue;
        }
        respawnEnemyFromTop(&enemies[i], i);
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

static s16 advanceVerticalScroll(void) {
    scrollAccumulator += scrollSpeedPerFrame;
    s16 scrollStep = 0;
    while (scrollAccumulator >= FIX16(1)) {
        scrollStep++;
        scrollAccumulator -= FIX16(1);
    }

    if (scrollStep > 0) {
        backgroundOffsetY -= scrollStep;
        if (backgroundOffsetY < 0) {
            backgroundOffsetY += SCROLL_LOOP_PX;
        }
    }

    return scrollStep;
}

static void applyBackgroundScroll(void) {
    if (mapBackground != NULL) {
        MAP_scrollTo(mapBackground, 0, backgroundOffsetY);
    }
}

static void resetChimneySpawnCursor(void) {
    nextChimneySpawnY = CHIMNEY_ROW_OFFSET;
}

static u8 rollChimneyProhibited(void) {
    u16 roll = random() % 100;
    return (roll < CHIMNEY_PROHIBITED_PERCENT) ? TRUE : FALSE;
}

static s16 takeNextChimneySpawnY(void) {
    s16 y = nextChimneySpawnY;
    u8 rowsAdvance = (random() & 1) ? 2 : 1; /* 1 o 2 casas */
    nextChimneySpawnY -= (rowsAdvance * HOUSE_HEIGHT);
    while (nextChimneySpawnY < -SCROLL_LOOP_PX) {
        nextChimneySpawnY += SCROLL_LOOP_PX;
    }
    return y;
}

static s16 pickChimneyX(void) {
    return (random() & 1) ? CHIMNEY_X_RIGHT : CHIMNEY_X_LEFT;
}

static void updateChimneys(s16 scrollStep) {
    for (u8 i = 0; i < NUM_CHIMNEYS; i++) {
        Chimney* chimney = &chimneys[i];
        if (!chimney->prohibited && chimney->state == CHIMNEY_COOLDOWN) {
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

        if (scrollStep) {
            chimney->y += scrollStep;
        }
        if (chimney->y > SCREEN_HEIGHT) {
            chimney->x = pickChimneyX();
            chimney->y = takeNextChimneySpawnY();
            chimney->prohibited = rollChimneyProhibited();
            chimney->state = chimney->prohibited ? CHIMNEY_INACTIVE : CHIMNEY_ACTIVE;
            chimney->cooldown = 0;
            chimney->blink = 0;
        }

        s16 screenX = chimney->x;
        u8 visible = (chimney->y + CHIMNEY_SIZE > 0) && (chimney->y < SCREEN_HEIGHT);
        if (chimney->sprite) {
            SPR_setPosition(chimney->sprite, screenX, chimney->y);
        }
        if (chimney->blockedSprite) {
            SPR_setPosition(chimney->blockedSprite, screenX, chimney->y);
        }

        if (chimney->prohibited) {
            if (chimney->blockedSprite) {
                SPR_setVisibility(chimney->blockedSprite, visible ? VISIBLE : HIDDEN);
            }
            if (chimney->sprite) {
                SPR_setVisibility(chimney->sprite, HIDDEN);
            }
            continue;
        }

        if (chimney->state == CHIMNEY_COOLDOWN) {
            if (chimney->sprite) {
                if (chimney->blink) {
                    SPR_setVisibility(chimney->sprite, HIDDEN);
                } else {
                    SPR_setVisibility(chimney->sprite, visible ? VISIBLE : HIDDEN);
                }
            }
            if (chimney->blockedSprite) {
                SPR_setVisibility(chimney->blockedSprite, HIDDEN);
            }
        } else {
            if (chimney->sprite) {
                SPR_setVisibility(chimney->sprite, visible ? VISIBLE : HIDDEN);
            }
            if (chimney->blockedSprite) {
                SPR_setVisibility(chimney->blockedSprite, HIDDEN);
            }
        }
    }
}

static void respawnEnemyFromTop(Enemy* enemy, u8 offsetIndex) {
    if (enemy == NULL) return;

    enemy->x = random() % (WORLD_WIDTH - ENEMY_WIDTH);
    enemy->y = -(ENEMY_HEIGHT + (offsetIndex * 40));
    enemy->vx = 0;
    enemy->vy = ENEMY_VERTICAL_SPEED;

    if (enemy->sprite) {
        SPR_setPosition(enemy->sprite, enemy->x, enemy->y);
        SPR_setVisibility(enemy->sprite, HIDDEN);
    }
}

static void updateEnemies(s16 scrollStep) {
    for (u8 i = 0; i < NUM_ENEMIES; i++) {
        Enemy* enemy = &enemies[i];
        if (!enemy->active || enemy->sprite == NULL) continue;

        s16 santaCenter = santa.x + (SANTA_WIDTH / 2);
        s16 enemyCenter = enemy->x + (ENEMY_WIDTH / 2);

        if (enemyCenter < santaCenter) {
            enemy->vx = ENEMY_HORIZONTAL_SPEED;
        } else if (enemyCenter > santaCenter) {
            enemy->vx = -ENEMY_HORIZONTAL_SPEED;
        } else {
            enemy->vx = 0;
        }

        enemy->x += enemy->vx;
        if (enemy->x < 0) {
            enemy->x = 0;
        } else if (enemy->x > (WORLD_WIDTH - ENEMY_WIDTH)) {
            enemy->x = WORLD_WIDTH - ENEMY_WIDTH;
        }

        enemy->y += enemy->vy;
        if (scrollStep) {
            enemy->y += scrollStep;
        }
        if (enemy->y > SCREEN_HEIGHT) {
            respawnEnemyFromTop(enemy, i);
        }

        const u8 visible = (enemy->y + ENEMY_HEIGHT > 0) && (enemy->y < SCREEN_HEIGHT);
        SPR_setPosition(enemy->sprite, enemy->x, enemy->y);
        SPR_setVisibility(enemy->sprite, visible ? VISIBLE : HIDDEN);
        SPR_setHFlip(enemy->sprite, (santaCenter < enemyCenter));
    }
}

static void updateGiftDrops(s16 scrollStep) {
    for (u8 i = 0; i < NUM_GIFT_DROPS; i++) {
        GiftDrop* drop = &drops[i];
        if (!drop->active || drop->sprite == NULL) continue;

        drop->y += drop->vy;
        if (scrollStep) {
            drop->y += scrollStep;
        }
        if (drop->y > SCREEN_HEIGHT) {
            drop->active = FALSE;
            SPR_setVisibility(drop->sprite, HIDDEN);
            continue;
        }

        SPR_setPosition(drop->sprite, drop->x, drop->y);
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
        SPR_setPosition(drop->sprite, drop->x, drop->y);
        SPR_setVisibility(drop->sprite, VISIBLE);
        break;
    }
}

static void handleGiftDrop(void) {
    if (giftsRemaining == 0) return;

    for (u8 i = 0; i < NUM_CHIMNEYS; i++) {
        Chimney* chimney = &chimneys[i];
        if (chimney->prohibited || chimney->state != CHIMNEY_ACTIVE) continue;

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
    /* Colisiones desactivadas temporalmente. */
    return;

    s16 santaHitX = santa.x + ((SANTA_WIDTH - SANTA_HITBOX_WIDTH) / 2);
    s16 santaHitY = santa.y + (SANTA_HEIGHT - SANTA_HITBOX_HEIGHT);

    for (u8 i = 0; i < NUM_ENEMIES; i++) {
        Enemy* enemy = &enemies[i];
        if (!enemy->active) continue;

        if (gameCore_checkCollision(santaHitX, santaHitY, SANTA_HITBOX_WIDTH, SANTA_HITBOX_HEIGHT,
                enemy->x, enemy->y, ENEMY_WIDTH, ENEMY_HEIGHT)) {
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
    SPR_setPosition(santa.sprite, santa.x, santa.y);
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

    return gameCore_checkCollision(santaHitX, santaHitY, SANTA_HITBOX_WIDTH, SANTA_HITBOX_HEIGHT,
        chimney->x, chimney->y, CHIMNEY_SIZE, CHIMNEY_SIZE);
}
