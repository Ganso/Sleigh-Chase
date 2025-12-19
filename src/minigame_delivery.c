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
#include "resources_bg.h"
#include "resources_sfx.h"
#include "resources_sprites.h"
#include "snow_effect.h"
#include "gift_counter.h"

#define DELIVERY_TARGET 10              /* Regalos totales a entregar en la fase. */
#define MAX_VISIBLE_CHIMNEYS 4          /* Máximo de chimeneas simultáneas en pantalla. */
#define NUM_CHIMNEYS MAX_VISIBLE_CHIMNEYS /* Pool de chimeneas reutilizado. */
#define MAX_ENEMIES 4                   /* Enemigos concurrentes en el tejado. */
#define NUM_GIFT_DROPS 3                /* Regalos simultáneos en caída. */
#define WORLD_WIDTH SCREEN_WIDTH        /* Ancho jugable fijado a la pantalla. */
#define WORLD_HEIGHT 512                /* Altura total del bucle vertical. */
#define SCROLL_LOOP_PX WORLD_HEIGHT     /* Tamaño del loop de scroll en píxeles. */
#define SCROLL_SPEED_PER_FRAME FIX16(2) /* Velocidad de avance vertical. */
#define CHIMNEY_SIZE 32                 /* Tamaño (ancho/alto) de cada chimenea. */
#define CHIMNEY_MARGIN_X 4              /* Margen lateral respecto al borde. */
#define CHIMNEY_X_LEFT CHIMNEY_MARGIN_X /* Posición X para chimeneas de la izquierda. */
#define CHIMNEY_X_RIGHT (SCREEN_WIDTH - CHIMNEY_SIZE - CHIMNEY_MARGIN_X) /* X derecha. */
#define CHIMNEY_PROHIBITED_PERCENT 30   /* Probabilidad % de chimenea prohibida. */
#define CHIMNEY_PRESET_LEFT_COUNT 5
#define CHIMNEY_PRESET_RIGHT_COUNT 5
#define CHIMNEY_HITBOX_OFFSET_X 6
#define CHIMNEY_HITBOX_OFFSET_Y 7
#define CHIMNEY_HITBOX_WIDTH 19
#define CHIMNEY_HITBOX_HEIGHT 17
#define ENEMY_WIDTH 32
#define ENEMY_HEIGHT 32
#define ENEMY_SPEED 3
#define ENEMY_CHASE_SPEED (ENEMY_SPEED * 2) /* mas rapido al perseguir regalos. */
#define ENEMY_DIR_CHANGE_MIN_FRAMES (2 * 60)
#define ENEMY_DIR_CHANGE_MAX_FRAMES (3 * 60)
#define ENEMY_STEAL_ANIM_FRAMES 5
#define ENEMY_HITBOX_OFFSET_X 5
#define ENEMY_HITBOX_OFFSET_Y 6
#define ENEMY_HITBOX_WIDTH 20
#define ENEMY_HITBOX_HEIGHT 22
#define DROP_COOLDOWN_FRAMES 18
#define CHIMNEY_RESET_FRAMES 90
#define CHIMNEY_TOGGLE_MIN_FRAMES (5 * 60)
#define CHIMNEY_TOGGLE_MAX_FRAMES (10 * 60)
#define RECOVERY_FRAMES 90
#define GIFT_FLY_SPEED 8

#define HUD_MARGIN_PX 5
#define GIFT_COUNTER_SPRITE_WIDTH 96
#define GIFT_COUNTER_SPRITE_HEIGHT 24
#define GIFT_COUNTER_ROW_OFFSET_Y 12
#define GIFT_COUNTER_SECOND_ROW_OFFSET_X 6
#define GIFT_COUNTER_ROW_SIZE 5

#define GIFT_SIZE 32
#define GIFT_HITBOX_OFFSET_X 2
#define GIFT_HITBOX_OFFSET_Y 2
#define GIFT_HITBOX_WIDTH 12
#define GIFT_HITBOX_HEIGHT 12
#define TARGET_MARK_SIZE 16
#define SANTA_THROW_SPAWN_FRAME 4
#define GIFT_COUNTER_BLINK_INTERVAL_FRAMES 3 /* Intervalo de parpadeo del HUD. */
#define GIFT_COUNTER_BLINK_DURATION_FRAMES 18 /* Duración total del parpadeo. */
#define SANTA_SPEED 6
#define SANTA_THROW_OFFSET_X 38
#define SANTA_THROW_OFFSET_Y 110
#define THROW_TARGET_RADIUS 150
#define THROW_FALLBACK_OFFSET_X 100

#define SANTA_WIDTH 80
#define SANTA_HEIGHT 128
#define SANTA_HITBOX_WIDTH 44
#define SANTA_HITBOX_HEIGHT 40
#define SANTA_START_Y (SCREEN_HEIGHT - SANTA_HEIGHT + 8)

#define DEPTH_HUD SPR_MIN_DEPTH
#define DEPTH_SANTA (SPR_MIN_DEPTH + 24)
#define DEPTH_EFFECTS (DEPTH_SANTA + 8)
#define DEPTH_BACKGROUND (DEPTH_EFFECTS + 8)
#define DEPTH_MARKERS (DEPTH_BACKGROUND - 2)

static const s16 chimneyLeftPresetY[CHIMNEY_PRESET_LEFT_COUNT] = { 30, 160, 270, 400, 436 };
static const s16 chimneyRightPresetY[CHIMNEY_PRESET_RIGHT_COUNT] = { 48, 82, 216, 324, 458 };

enum {
    CHIMNEY_INACTIVE = 0,
    CHIMNEY_ACTIVE = 1,
    CHIMNEY_COOLDOWN = 2,
};

typedef struct {
    Sprite* sprite;
    Sprite* blockedSprite;
    Sprite* usedSprite;
    s16 x;
    s16 y;
    u8 state;
    u16 cooldown;
    u8 blink;
    u8 prohibited;
    u16 toggleTimer;
} Chimney;

typedef struct {
    Sprite* sprite;
    s16 x;
    s16 y;
    s8 vx;
    s8 vy;
    fix16 fx;
    fix16 fy;
    fix16 fvx;
    fix16 fvy;
    u8 active;
    u8 stealAnimTimer;
    u16 directionTimer;
} Enemy;

typedef struct {
    Sprite* sprite;
    Sprite* targetSprite;
    s16 x;
    s16 y;
    s16 targetX;
    s16 targetY;
    fix16 fx;
    fix16 fy;
    fix16 vx;
    fix16 vy;
    u16 framesToTarget;
    u8 active;
    u8 pending;
} GiftDrop;

typedef struct {
    Sprite* sprite;
    s16 x;
    s16 y;
    s8 vx;
    s8 vy;
} Santa;
static Chimney chimneys[NUM_CHIMNEYS]; /**< Chimeneas activas en el tejado. */
static Enemy enemies[MAX_ENEMIES]; /**< Enemigos que roban regalos. */
static GiftDrop drops[NUM_GIFT_DROPS]; /**< Regalos en caída hacia chimeneas. */
static Santa santa; /**< Control del sprite principal. */
static u8 santaThrowing; /**< TRUE mientras Santa ejecuta la animación de lanzamiento. */
static u8 santaThrowGiftSpawned; /**< TRUE cuando el regalo ya se generó en la animación. */
static u8 santaReturnToIdle; /**< Solicitud de volver a la animación base. */

static Map* mapBackground; /**< Mapa del plano B para el tejado. */
static s16 backgroundOffsetY; /**< Offset vertical del scroll de fondo. */
static fix16 backgroundOffsetFY; /**< Offset vertical de scroll en fix16. */
static fix16 scrollSpeedPerFrame; /**< Velocidad de scroll por frame. */
static SnowEffect snowEffect; /**< Efecto de nieve compartido. */
static GameTimer gameTimer; /**< Temporizador de la fase para derrota. */

static Sprite* giftCounterTop; /**< Contador gráfico fila superior. */
static Sprite* giftCounterBottom; /**< Contador gráfico fila inferior. */
static GiftCounterHUD giftCounterHUD; /**< Configuración del contador de regalos. */
static GiftCounterBlink giftCounterBlink; /**< Parpadeo compartido para el HUD. */
static u16 giftCounterBlinkStartFrame; /**< Momento en el que comenz¢ el parpadeo. */
static u16 lastGiftCounterDisplay; /**< Último valor pintado en el HUD. */

static u16 frameCounter; /**< Contador global de frames. */
static GameInertia santaInertia; /**< Inercia de movimiento para Santa (fase 2). */
static u8 giftCounterValue; /**< Contador de regalos entregados/fallidos como en fase 1. */
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
static void applyBackgroundScroll(s16 scrollStep);
static void updateChimneys(s16 scrollStep);
static void updateEnemies(s16 scrollStep);
static void updateGiftDrops(s16 scrollStep);
static void updateSantaThrowState(void);
static void respawnEnemyFromTop(Enemy* enemy, u8 offsetIndex);
static s16 pickChimneyPresetY(u8 spawnRight);
static s16 mapYToScreenY(s16 mapY, u8 spawnAboveTop);
static u8 chimneysOverlapAt(s16 x, s16 y, u8 selfIndex);
static void placeChimneyAtPreset(Chimney* chimney, u8 index, u8 spawnAboveTop);
static u8 rollChimneyProhibited(void);
static u16 rollChimneyToggleFrames(void);
static u16 rollEnemyDirectionTimer(void);
static void updateGiftCounter(void);
static void spawnGiftDrop(void);
static void startGiftThrow(void);
static void resolveGiftDropAtTarget(const GiftDrop* drop);
static Chimney* findChimneyAtPoint(s16 x, s16 y);
static Chimney* findAnyChimneyAtPoint(s16 x, s16 y);
static s16 abs16(s16 value);
static void checkEnemyCollision(void);
static void startRecovery(void);
static void updateRecovery(void);
static void onSantaFrameChange(Sprite* sprite);
static void updateEnemyActivation(void);
static u8 getTargetEnemyCount(void);
static void deactivateGiftDrop(GiftDrop* drop);
static u8 checkGiftEnemyCollision(GiftDrop* drop);
static u8 getActiveGiftTargetPos(s16* targetX, s16* targetY);
static Chimney* findNearestChimneyInRange(s16 centerX, s16 centerY, u16 radius, s32* outDistanceSq);
static void onGiftSuccess(void);
static void playRandomElfStealSound(void);
static void playGiftDeliveredSound(void);
static void playGiftLostSound(void);

/** @brief Configura recursos, estado inicial de la fase. */
void minigameDelivery_init(void) {
    gameCore_resetVideoState();
    // kprintf("[SANTA] starting Santa init at pos=(%d,%d)", (WORLD_WIDTH - SANTA_WIDTH) / 2, SANTA_START_Y);

    frameCounter = 0;
    giftCounterValue = 0;
    deliveriesCompleted = 0;
    phaseCompleted = FALSE;
    dropCooldown = 0;
    recoveringFrames = 0;
    previousInput = 0;
    santaThrowing = FALSE;
    santaThrowGiftSpawned = FALSE;
    santaReturnToIdle = FALSE;
    backgroundOffsetY = SCROLL_LOOP_PX;
    backgroundOffsetY %= SCROLL_LOOP_PX;
    backgroundOffsetFY = FIX16(backgroundOffsetY);
    scrollSpeedPerFrame = SCROLL_SPEED_PER_FRAME;
    santaInertia.accel = 3;
    santaInertia.friction = 3;
    santaInertia.frictionDelay = 1;
    santaInertia.maxVelocity = 10;

    
    initBackground();
    initSanta();
    initChimneys();
    initEnemies();
    initGiftDrops();
    initGiftCounterSprites();
    updateEnemyActivation();
    updateGiftCounter();

    giftCounterBlinkStartFrame = 0;
    gameCore_initTimer(&gameTimer, 0);

    XGM2_playPCM(snd_santa_hohoho, sizeof(snd_santa_hohoho), SOUND_PCM_CH_AUTO);
}

/** @brief Actualiza entrada, movimiento y lógica principal de la fase. */
void minigameDelivery_update(void) {
    frameCounter++;

    snowEffect_update(&snowEffect, frameCounter);

    u16 input = gameCore_readInput();
    updateSantaThrowState();

    static u8 santaMissingLogged = FALSE;
    if (santa.sprite == NULL && !santaMissingLogged) {
        // kprintf("[SANTA][ERROR] missing sprite at update frame=%u", frameCounter);
        santaMissingLogged = TRUE;
    } else if (santa.sprite && (frameCounter % 120) == 0) {
        // kprintf("[SANTA] frame=%u pos=(%d,%d) depth=%d visible_check", frameCounter, santa.x, santa.y, DEPTH_SANTA);
    }

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
            dirX, dirY, 0, WORLD_WIDTH - SANTA_WIDTH,
            0, SCREEN_HEIGHT - SANTA_HEIGHT,
            frameCounter, &santaInertia);

        if (dropCooldown > 0) {
            dropCooldown--;
        }

        const u8 pressedA = (input & BUTTON_A) && !(previousInput & BUTTON_A);

        if (pressedA) {
            startGiftThrow();
        }
    }

    s16 scrollStep = advanceVerticalScroll();
    applyBackgroundScroll(scrollStep);
    updateChimneys(scrollStep);
    updateEnemies(scrollStep);
    updateGiftDrops(scrollStep);

    SPR_setPosition(santa.sprite, santa.x, santa.y);

    if (recoveringFrames == 0) {
        checkEnemyCollision();
    }

    gameCore_updateTimer(&gameTimer);
    updateGiftCounter();

    if (giftCounterValue >= DELIVERY_TARGET) {
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

/** @brief Libera el mapa de fondo y evita fugas entre fases. */
void minigameDelivery_shutdown(void) {
    if (mapBackground) {
        MAP_release(mapBackground);
        mapBackground = NULL;
    }
}

static void initBackground(void) {
    gameCore_resetTileIndex();

    backgroundOffsetFY = FIX16(backgroundOffsetY);
    scrollSpeedPerFrame = SCROLL_SPEED_PER_FRAME;

    if (image_fondo_tejados_pal.data) {
        PAL_setPalette(PAL_COMMON, image_fondo_tejados_pal.data, CPU);
    }
    if (sprite_santa_car_volando.palette) {
        PAL_setPalette(PAL_PLAYER, sprite_santa_car_volando.palette->data, CPU);
    } else if (sprite_santa_car.palette) {
        PAL_setPalette(PAL_PLAYER, sprite_santa_car.palette->data, CPU);
    }
    if (sprite_duende_malo_volador.palette) {
        PAL_setPalette(PAL_EFFECT, sprite_duende_malo_volador.palette->data, CPU);
    }

    VDP_setBackgroundColor(0);

    mapBackground = gameCore_loadMapWithTiles(&image_fondo_tejados_tile, &image_fondo_tejados_map,
        BG_B, PAL_COMMON, FALSE, FALSE, FALSE, &globalTileIndex);
    if (mapBackground != NULL) {
        MAP_scrollTo(mapBackground, 0, backgroundOffsetY);
    }
    SYS_doVBlankProcess();

    snowEffect_init(&snowEffect, &globalTileIndex, 2, -8);

}

static void initSanta(void) {
    santa.x = (WORLD_WIDTH - SANTA_WIDTH) / 2;
    santa.y = SANTA_START_Y;
    santa.vx = 0;
    santa.vy = 0;

    // kprintf("[SANTA] starting Santa init at pos=(%d,%d)", santa.x, santa.y);
    santa.sprite = SPR_addSpriteSafe(&sprite_santa_car_volando, santa.x, santa.y,
        TILE_ATTR(PAL_PLAYER, FALSE, FALSE, FALSE));
    if (santa.sprite) {
        // kprintf("[SANTA] sprite created successfully");
        SPR_setDepth(santa.sprite, DEPTH_SANTA);
        SPR_setAutoAnimation(santa.sprite, TRUE);
        SPR_setAnimationLoop(santa.sprite, TRUE);
        SPR_setFrameChangeCallback(santa.sprite, onSantaFrameChange);
        SPR_setVisibility(santa.sprite, VISIBLE);
        // kprintf("[SANTA] sprite ok depth=%d pos=(%d,%d)", DEPTH_SANTA, santa.x, santa.y);
    } else {
        // kprintf("[SANTA][ERROR] sprite not created");
    }
}

static void initChimneys(void) {
    memset(chimneys, 0, sizeof(chimneys));
    for (u8 i = 0; i < NUM_CHIMNEYS; i++) {
        chimneys[i].state = CHIMNEY_ACTIVE;
        chimneys[i].cooldown = 0;
        chimneys[i].blink = 0;
        chimneys[i].prohibited = rollChimneyProhibited();
        chimneys[i].toggleTimer = rollChimneyToggleFrames();
        placeChimneyAtPreset(&chimneys[i], i, TRUE);

        chimneys[i].sprite = SPR_addSpriteSafe(&sprite_chimenea,
            chimneys[i].x, chimneys[i].y,
            TILE_ATTR(PAL_COMMON, FALSE, FALSE, FALSE));
        chimneys[i].blockedSprite = SPR_addSpriteSafe(&sprite_chimenea_prohibida,
            chimneys[i].x, chimneys[i].y,
            TILE_ATTR(PAL_COMMON, FALSE, FALSE, FALSE));
        chimneys[i].usedSprite = SPR_addSpriteSafe(&sprite_chimenea_utilizada,
            chimneys[i].x, chimneys[i].y,
            TILE_ATTR(PAL_COMMON, FALSE, FALSE, FALSE));
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
        if (chimneys[i].usedSprite) {
            SPR_setDepth(chimneys[i].usedSprite, DEPTH_BACKGROUND);
            SPR_setAutoAnimation(chimneys[i].usedSprite, TRUE);
            SPR_setAnimationLoop(chimneys[i].usedSprite, TRUE);
            SPR_setVisibility(chimneys[i].usedSprite, HIDDEN);
        }
    }
}

static void initEnemies(void) {
    memset(enemies, 0, sizeof(enemies));
    for (u8 i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = FALSE;
        enemies[i].vx = 0;
        enemies[i].vy = 0;
        enemies[i].stealAnimTimer = 0;
        enemies[i].directionTimer = rollEnemyDirectionTimer();
        enemies[i].sprite = SPR_addSpriteSafe(&sprite_duende_malo_volador, 0, 0,
            TILE_ATTR(PAL_EFFECT, FALSE, FALSE, FALSE));
        if (enemies[i].sprite) {
            SPR_setDepth(enemies[i].sprite, DEPTH_EFFECTS);
            SPR_setAutoAnimation(enemies[i].sprite, TRUE);
        } else {
            continue;
        }
        respawnEnemyFromTop(&enemies[i], i);
    }
}

static void initGiftDrops(void) {
    memset(drops, 0, sizeof(drops));
    for (u8 i = 0; i < NUM_GIFT_DROPS; i++) {
        drops[i].sprite = SPR_addSpriteSafe(&sprite_regalo, 0, 0,
            TILE_ATTR(PAL_EFFECT, FALSE, FALSE, FALSE));
        drops[i].active = FALSE;
        drops[i].targetX = 0;
        drops[i].targetY = 0;
        drops[i].fx = FIX16(0);
        drops[i].fy = FIX16(0);
        drops[i].vx = FIX16(0);
        drops[i].vy = FIX16(0);
        drops[i].framesToTarget = 0;
        drops[i].pending = FALSE;
        drops[i].targetSprite = SPR_addSpriteSafe(&sprite_marca_x_2, 0, 0,
            TILE_ATTR(PAL_PLAYER, FALSE, FALSE, FALSE));
        if (drops[i].sprite) {
            SPR_setDepth(drops[i].sprite, DEPTH_EFFECTS);
            SPR_setAutoAnimation(drops[i].sprite, FALSE);
            SPR_setAnim(drops[i].sprite, 0);
            SPR_setVisibility(drops[i].sprite, HIDDEN);
        }
        if (drops[i].targetSprite) {
            SPR_setDepth(drops[i].targetSprite, DEPTH_MARKERS);
            SPR_setVisibility(drops[i].targetSprite, HIDDEN);
        }
    }
}

static void initGiftCounterSprites(void) {
    const s16 baseX = SCREEN_WIDTH - HUD_MARGIN_PX - GIFT_COUNTER_SPRITE_WIDTH;
    const s16 baseY = SCREEN_HEIGHT - GIFT_COUNTER_SPRITE_HEIGHT - HUD_MARGIN_PX;

    giftCounterTop = SPR_addSpriteSafe(&sprite_icono_regalo, baseX, baseY - GIFT_COUNTER_ROW_OFFSET_Y,
        TILE_ATTR(PAL_EFFECT, FALSE, FALSE, FALSE));
    if (giftCounterTop) {
        SPR_setAutoAnimation(giftCounterTop, FALSE);
        SPR_setDepth(giftCounterTop, DEPTH_HUD + 1);
        SPR_setFrame(giftCounterTop, 0);
        SPR_setVisibility(giftCounterTop, VISIBLE);
        // kprintf("[HUD] giftCounterTop pos=(%d,%d)", baseX, baseY - GIFT_COUNTER_ROW_OFFSET_Y);
    }

    giftCounterBottom = SPR_addSpriteSafe(&sprite_icono_regalo, baseX + GIFT_COUNTER_SECOND_ROW_OFFSET_X, baseY,
        TILE_ATTR(PAL_EFFECT, FALSE, FALSE, FALSE));
    if (giftCounterBottom) {
        SPR_setAutoAnimation(giftCounterBottom, FALSE);
        SPR_setDepth(giftCounterBottom, DEPTH_HUD);
        SPR_setFrame(giftCounterBottom, 0);
        SPR_setVisibility(giftCounterBottom, VISIBLE);
        // kprintf("[HUD] giftCounterBottom pos=(%d,%d)", baseX + GIFT_COUNTER_SECOND_ROW_OFFSET_X, baseY);
    }

    giftCounter_initHUD(&giftCounterHUD, giftCounterTop, giftCounterBottom,
        baseX, baseY, -GIFT_COUNTER_ROW_OFFSET_Y, GIFT_COUNTER_SECOND_ROW_OFFSET_X,
        DEPTH_HUD + 1, DEPTH_HUD, GIFT_COUNTER_ROW_SIZE, DELIVERY_TARGET);
    giftCounter_stopBlink(&giftCounterBlink);
    lastGiftCounterDisplay = 0xFFFF;
}

static s16 advanceVerticalScroll(void) {
    const s16 previousOffsetY = backgroundOffsetY;

    backgroundOffsetFY -= scrollSpeedPerFrame;
    if (backgroundOffsetFY < 0) {
        backgroundOffsetFY += FIX16(SCROLL_LOOP_PX);
    }

    backgroundOffsetY = F16_toInt(backgroundOffsetFY);
    s16 scrollStep = previousOffsetY - backgroundOffsetY;
    if (scrollStep < 0) {
        scrollStep += SCROLL_LOOP_PX;
    }

    return scrollStep;
}

static void applyBackgroundScroll(s16 scrollStep) {
    if (mapBackground != NULL && scrollStep > 0) {
        MAP_scrollTo(mapBackground, 0, backgroundOffsetY);
    }
}

static s16 mapYToScreenY(s16 mapY, u8 spawnAboveTop) {
    s16 screenY = mapY - backgroundOffsetY;

    while (screenY <= -CHIMNEY_SIZE) {
        screenY += SCROLL_LOOP_PX;
    }
    while (screenY > SCREEN_HEIGHT) {
        screenY -= SCROLL_LOOP_PX;
    }

    if (spawnAboveTop && screenY > -CHIMNEY_SIZE) {
        screenY -= SCROLL_LOOP_PX;
    }

    return screenY;
}

static s16 pickChimneyPresetY(u8 spawnRight) {
    const s16* list = spawnRight ? chimneyRightPresetY : chimneyLeftPresetY;
    const u8 count = spawnRight ? CHIMNEY_PRESET_RIGHT_COUNT : CHIMNEY_PRESET_LEFT_COUNT;
    const u8 index = random() % count;
    return list[index];
}

static u8 chimneysOverlapAt(s16 x, s16 y, u8 selfIndex) {
    for (u8 i = 0; i < NUM_CHIMNEYS; i++) {
        if (i == selfIndex) continue;
        const Chimney* other = &chimneys[i];
        if (other->state == CHIMNEY_INACTIVE) continue;

        const u8 overlap =
            (x < (other->x + CHIMNEY_SIZE)) &&
            ((x + CHIMNEY_SIZE) > other->x) &&
            (y < (other->y + CHIMNEY_SIZE)) &&
            ((y + CHIMNEY_SIZE) > other->y);
        if (overlap) {
            return TRUE;
        }
    }

    return FALSE;
}

static void placeChimneyAtPreset(Chimney* chimney, u8 index, u8 spawnAboveTop) {
    if (chimney == NULL) return;

    const u8 maxAttempts = 12;
    for (u8 attempt = 0; attempt < maxAttempts; attempt++) {
        const u8 spawnRight = random() & 1;
        const s16 mapY = pickChimneyPresetY(spawnRight);
        const s16 screenY = mapYToScreenY(mapY, spawnAboveTop);
        const s16 x = spawnRight ? CHIMNEY_X_RIGHT : CHIMNEY_X_LEFT;

        if (!chimneysOverlapAt(x, screenY, index)) {
            chimney->x = x;
            chimney->y = screenY;
            return;
        }
    }

    for (u8 side = 0; side < 2; side++) {
        const u8 spawnRight = (side != 0);
        const s16* list = spawnRight ? chimneyRightPresetY : chimneyLeftPresetY;
        const u8 count = spawnRight ? CHIMNEY_PRESET_RIGHT_COUNT : CHIMNEY_PRESET_LEFT_COUNT;
        for (u8 i = 0; i < count; i++) {
            const s16 mapY = list[i];
            const s16 screenY = mapYToScreenY(mapY, spawnAboveTop);
            const s16 x = spawnRight ? CHIMNEY_X_RIGHT : CHIMNEY_X_LEFT;
            if (!chimneysOverlapAt(x, screenY, index)) {
                chimney->x = x;
                chimney->y = screenY;
                return;
            }
        }
    }

    chimney->x = CHIMNEY_X_LEFT;
    chimney->y = mapYToScreenY(chimneyLeftPresetY[0], spawnAboveTop);
}

static u8 rollChimneyProhibited(void) {
    u16 roll = random() % 100;
    return (roll < CHIMNEY_PROHIBITED_PERCENT) ? TRUE : FALSE;
}

static u16 rollChimneyToggleFrames(void) {
    const u16 minFrames = CHIMNEY_TOGGLE_MIN_FRAMES;
    const u16 maxFrames = CHIMNEY_TOGGLE_MAX_FRAMES;
    const u16 span = (maxFrames > minFrames) ? (maxFrames - minFrames) : 0;
    const u16 randomOffset = span ? (random() % (span + 1)) : 0;
    return minFrames + randomOffset;
}

static void updateChimneys(s16 scrollStep) {
    for (u8 i = 0; i < NUM_CHIMNEYS; i++) {
        Chimney* chimney = &chimneys[i];

        if ((chimney->state != CHIMNEY_COOLDOWN) && chimney->toggleTimer > 0) {
            chimney->toggleTimer--;
            if (chimney->toggleTimer == 0) {
                chimney->prohibited = !chimney->prohibited;
                chimney->toggleTimer = rollChimneyToggleFrames();
            }
        }

        if (chimney->state == CHIMNEY_COOLDOWN) {
            if (chimney->cooldown > 0) {
                chimney->cooldown--;
            }
        }

        if (scrollStep) {
            chimney->y += scrollStep;
        }
        if (chimney->y > SCREEN_HEIGHT) {
            placeChimneyAtPreset(chimney, i, TRUE);
            chimney->prohibited = rollChimneyProhibited();
            chimney->state = CHIMNEY_ACTIVE;
            chimney->cooldown = 0;
            chimney->blink = 0;
            chimney->toggleTimer = rollChimneyToggleFrames();
            if (chimney->usedSprite) {
                SPR_setVisibility(chimney->usedSprite, HIDDEN);
            }
        }

        s16 screenX = chimney->x;
        u8 visible = (chimney->y + CHIMNEY_SIZE > 0) && (chimney->y < SCREEN_HEIGHT);
        if (chimney->sprite) {
            SPR_setPosition(chimney->sprite, screenX, chimney->y);
        }
        if (chimney->blockedSprite) {
            SPR_setPosition(chimney->blockedSprite, screenX, chimney->y);
        }
        if (chimney->usedSprite) {
            SPR_setPosition(chimney->usedSprite, screenX, chimney->y);
        }

        if (chimney->prohibited) {
            if (chimney->blockedSprite) {
                SPR_setVisibility(chimney->blockedSprite, visible ? VISIBLE : HIDDEN);
            }
            if (chimney->sprite) {
                SPR_setVisibility(chimney->sprite, HIDDEN);
            }
            if (chimney->usedSprite) {
                SPR_setVisibility(chimney->usedSprite, HIDDEN);
            }
            continue;
        }

        if (chimney->state == CHIMNEY_COOLDOWN) {
            /* Mantener visible durante cooldown para que no desaparezca. */
            if (chimney->usedSprite) {
                SPR_setVisibility(chimney->usedSprite, visible ? VISIBLE : HIDDEN);
                if (chimney->sprite) {
                    SPR_setVisibility(chimney->sprite, HIDDEN);
                }
            } else if (chimney->sprite) {
                SPR_setVisibility(chimney->sprite, visible ? VISIBLE : HIDDEN);
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
            if (chimney->usedSprite) {
                SPR_setVisibility(chimney->usedSprite, HIDDEN);
            }
        }
    }
}

static void respawnEnemyFromTop(Enemy* enemy, u8 offsetIndex) {
    if (enemy == NULL) return;

    enemy->x = random() % (WORLD_WIDTH - ENEMY_WIDTH);
    enemy->y = -(ENEMY_HEIGHT + (offsetIndex * 20));
    enemy->vx = 0;
    enemy->vy = ENEMY_SPEED;
    enemy->fx = FIX16(enemy->x);
    enemy->fy = FIX16(enemy->y);
    enemy->fvx = FIX16(enemy->vx);
    enemy->fvy = FIX16(enemy->vy);
    enemy->stealAnimTimer = 0;
    enemy->directionTimer = rollEnemyDirectionTimer();

    if (enemy->sprite) {
        SPR_setPosition(enemy->sprite, enemy->x, enemy->y);
        SPR_setVisibility(enemy->sprite, HIDDEN);
        SPR_setAnim(enemy->sprite, 0);
        SPR_setAnimationLoop(enemy->sprite, TRUE);
        SPR_setAutoAnimation(enemy->sprite, TRUE);
    }

    /* Sonido de aparición desactivado temporalmente. */
}

static void updateEnemies(s16 scrollStep) {
    s16 targetX = 0;
    s16 targetY = 0;
    const u8 hasGiftTarget = getActiveGiftTargetPos(&targetX, &targetY);
    s8 nearestIndex = -1;
    if (hasGiftTarget) {
        u32 bestDist = 0xFFFFFFFF;
        for (u8 i = 0; i < MAX_ENEMIES; i++) {
            Enemy* enemy = &enemies[i];
            if (!enemy->active || enemy->sprite == NULL) continue;
            const s16 enemyCenterX = enemy->x + (ENEMY_WIDTH / 2);
            const s16 enemyCenterY = enemy->y + (ENEMY_HEIGHT / 2);
            const s32 dx = (s32)enemyCenterX - (s32)targetX;
            const s32 dy = (s32)enemyCenterY - (s32)targetY;
            const u32 dist = (u32)(dx * dx + dy * dy);
            if (dist < bestDist) {
                bestDist = dist;
                nearestIndex = (s8)i;
            }
        }
    }

    for (u8 i = 0; i < MAX_ENEMIES; i++) {
        Enemy* enemy = &enemies[i];
        if (!enemy->active || enemy->sprite == NULL) continue;

        (void)scrollStep; /* El enemigo ya no depende del scroll vertical. */
        u8 resumePatrol = FALSE;

        if (enemy->stealAnimTimer > 0) {
            enemy->stealAnimTimer--;
            if (enemy->stealAnimTimer == 0) {
                SPR_setAnim(enemy->sprite, 0);
                SPR_setAnimationLoop(enemy->sprite, TRUE);
                SPR_setAutoAnimation(enemy->sprite, TRUE);
                /* Tras robar, vuelve a patrullar con velocidad base y direcci¢n aleatoria. */
                s8 dirX = 0;
                s8 dirY = 0;
                do {
                    dirX = (random() % 3) - 1;
                    dirY = (random() % 3) - 1;
                } while (dirX == 0 && dirY == 0);

                enemy->vx = dirX * ENEMY_SPEED;
                enemy->vy = dirY * ENEMY_SPEED;
                enemy->directionTimer = rollEnemyDirectionTimer();
                resumePatrol = TRUE;
            }
        }

        u8 chaseGift = hasGiftTarget && ((s8)i == nearestIndex);
        if (resumePatrol) {
            chaseGift = FALSE;
        }
        if (chaseGift) {
            const s16 enemyCenterX = enemy->x + (ENEMY_WIDTH / 2);
            const s16 enemyCenterY = enemy->y + (ENEMY_HEIGHT / 2);
            if (enemyCenterX < targetX) {
                enemy->vx = ENEMY_CHASE_SPEED;
            } else if (enemyCenterX > targetX) {
                enemy->vx = -ENEMY_CHASE_SPEED;
            } else {
                enemy->vx = 0;
            }

            if (enemyCenterY < targetY) {
                enemy->vy = ENEMY_CHASE_SPEED;
            } else if (enemyCenterY > targetY) {
                enemy->vy = -ENEMY_CHASE_SPEED;
            } else {
                enemy->vy = 0;
            }
        } else {
            if (enemy->directionTimer > 0) {
                enemy->directionTimer--;
            }
            if (enemy->directionTimer == 0) {
                /* Elegir nueva dirección aleatoria, evitando vector nulo. */
                s8 dirX = 0;
                s8 dirY = 0;
                do {
                    dirX = (random() % 3) - 1; /* -1, 0 o 1 */
                    dirY = (random() % 3) - 1; /* -1, 0 o 1 */
                } while (dirX == 0 && dirY == 0);

                enemy->vx = dirX * ENEMY_SPEED;
                enemy->vy = dirY * ENEMY_SPEED;
                enemy->directionTimer = rollEnemyDirectionTimer();
            }
        }

        enemy->fvx = FIX16(enemy->vx);
        enemy->fvy = FIX16(enemy->vy);
        enemy->fx += enemy->fvx;
        enemy->fy += enemy->fvy;
        enemy->x = F16_toInt(enemy->fx);
        enemy->y = F16_toInt(enemy->fy);

        if (enemy->x < 0) {
            enemy->x = 0;
            enemy->vx = ENEMY_SPEED;
            enemy->fx = FIX16(enemy->x);
            enemy->fvx = FIX16(enemy->vx);
        } else if (enemy->x > (WORLD_WIDTH - ENEMY_WIDTH)) {
            enemy->x = WORLD_WIDTH - ENEMY_WIDTH;
            enemy->vx = -ENEMY_SPEED;
            enemy->fx = FIX16(enemy->x);
            enemy->fvx = FIX16(enemy->vx);
        }

        if (enemy->y < 0) {
            enemy->y = 0;
            enemy->vy = ENEMY_SPEED;
            enemy->fy = FIX16(enemy->y);
            enemy->fvy = FIX16(enemy->vy);
        } else if (enemy->y > (SCREEN_HEIGHT - ENEMY_HEIGHT)) {
            enemy->y = SCREEN_HEIGHT - ENEMY_HEIGHT;
            enemy->vy = -ENEMY_SPEED;
            enemy->fy = FIX16(enemy->y);
            enemy->fvy = FIX16(enemy->vy);
        }

        const u8 visible = (enemy->y + ENEMY_HEIGHT > 0) && (enemy->y < SCREEN_HEIGHT);
        SPR_setPosition(enemy->sprite, enemy->x, enemy->y);
        SPR_setVisibility(enemy->sprite, visible ? VISIBLE : HIDDEN);
        SPR_setHFlip(enemy->sprite, (enemy->fvx < 0));
    }
}

static u8 getTargetEnemyCount(void) {
    u8 count = 1;
    if (giftCounterValue >= 2) count++;
    if (giftCounterValue >= 4) count++;
    if (giftCounterValue >= 6) count++;
    if (count > MAX_ENEMIES) count = MAX_ENEMIES;
    return count;
}

static void updateEnemyActivation(void) {
    const u8 desiredEnemies = getTargetEnemyCount();

    for (u8 i = 0; i < MAX_ENEMIES; i++) {
        Enemy* enemy = &enemies[i];
        const u8 shouldBeActive = (i < desiredEnemies);

        if (shouldBeActive && !enemy->active) {
            enemy->active = TRUE;
            respawnEnemyFromTop(enemy, i);
            if (enemy->sprite) {
                SPR_setVisibility(enemy->sprite, VISIBLE);
            }
        } else if (!shouldBeActive && enemy->active) {
            enemy->active = FALSE;
            if (enemy->sprite) {
                SPR_setVisibility(enemy->sprite, HIDDEN);
            }
        }
    }
}

static void deactivateGiftDrop(GiftDrop* drop) {
    drop->active = FALSE;
    drop->pending = FALSE;
    if (drop->sprite) {
        SPR_setVisibility(drop->sprite, HIDDEN);
    }
    if (drop->targetSprite) {
        SPR_setVisibility(drop->targetSprite, HIDDEN);
    }
}

static u8 checkGiftEnemyCollision(GiftDrop* drop) {
    if (drop == NULL || !drop->active) return FALSE;

    for (u8 i = 0; i < MAX_ENEMIES; i++) {
        Enemy* enemy = &enemies[i];
        if (!enemy->active) continue;

        const s16 giftHitX = drop->x + GIFT_HITBOX_OFFSET_X;
        const s16 giftHitY = drop->y + GIFT_HITBOX_OFFSET_Y;
        const s16 enemyHitX = enemy->x + ENEMY_HITBOX_OFFSET_X;
        const s16 enemyHitY = enemy->y + ENEMY_HITBOX_OFFSET_Y;

        if (gameCore_checkCollision(giftHitX, giftHitY, GIFT_HITBOX_WIDTH, GIFT_HITBOX_HEIGHT,
                enemyHitX, enemyHitY, ENEMY_HITBOX_WIDTH, ENEMY_HITBOX_HEIGHT)) {
            // kprintf("[THROW] gift captured by enemy idx=%u at (%d,%d)", i, drop->x, drop->y);
            playRandomElfStealSound();
            enemy->stealAnimTimer = ENEMY_STEAL_ANIM_FRAMES;
            if (enemy->sprite) {
                SPR_setAnim(enemy->sprite, 1);
                SPR_setAnimationLoop(enemy->sprite, FALSE);
                SPR_setAutoAnimation(enemy->sprite, TRUE);
            }
            deactivateGiftDrop(drop);
            /* Entregas robadas ya no afectan al contador. */
            /* onGiftFailure(); */
            return TRUE;
        }
    }

    return FALSE;
}

static void updateGiftDrops(s16 scrollStep) {
    for (u8 i = 0; i < NUM_GIFT_DROPS; i++) {
        GiftDrop* drop = &drops[i];
        const u8 hasTarget = drop->pending || drop->active;
        if (!hasTarget) continue;

        if (scrollStep) {
            drop->targetY += scrollStep;
            if (drop->active) {
                drop->fy += FIX16(scrollStep);
            }
        }

        if (drop->active && drop->framesToTarget > 0) {
            drop->framesToTarget--;
        }

        if (drop->active && drop->sprite) {
            drop->fx += drop->vx;
            drop->fy += drop->vy;

            drop->x = F16_toInt(drop->fx);
            drop->y = F16_toInt(drop->fy);

            SPR_setPosition(drop->sprite, drop->x, drop->y);
            SPR_setVisibility(drop->sprite, VISIBLE);

            if (checkGiftEnemyCollision(drop)) {
                continue;
            }
        }

        if (drop->targetSprite) {
            const s16 markOffset = (GIFT_SIZE - TARGET_MARK_SIZE) / 2;
            const s16 targetMarkX = drop->targetX + markOffset;
            const s16 targetMarkY = drop->targetY + markOffset;
            SPR_setDepth(drop->targetSprite, DEPTH_MARKERS);
            SPR_setPosition(drop->targetSprite, targetMarkX, targetMarkY);
            SPR_setVisibility(drop->targetSprite, VISIBLE);
        }

        if (!drop->active) {
            continue;
        }

        if (drop->active && drop->sprite) {
            const u8 arrivedByFrames = (drop->framesToTarget == 0);
            const u8 closeEnough = (abs16(drop->x - drop->targetX) <= 1) && (abs16(drop->y - drop->targetY) <= 1);
            if (arrivedByFrames || closeEnough) {
                drop->x = drop->targetX;
                drop->y = drop->targetY;
                drop->fx = FIX16(drop->x);
                drop->fy = FIX16(drop->y);
                SPR_setPosition(drop->sprite, drop->x, drop->y);
                resolveGiftDropAtTarget(drop);
                deactivateGiftDrop(drop);
            }
        }
    }
}

static s16 abs16(s16 value) {
    return (value < 0) ? -value : value;
}

static void updateGiftCounter(void) {
    if (giftCounterBlink.active) {
        const u16 elapsed = frameCounter - giftCounterBlinkStartFrame;
        if (elapsed >= GIFT_COUNTER_BLINK_DURATION_FRAMES) {
            giftCounter_stopBlink(&giftCounterBlink);
        }
    }

    const u16 displayValue = giftCounter_getDisplayValue(&giftCounterBlink,
        giftCounterValue, frameCounter);
    if (displayValue != lastGiftCounterDisplay) {
        giftCounter_render(&giftCounterHUD, displayValue);
        lastGiftCounterDisplay = displayValue;
    }
}

static u16 rollEnemyDirectionTimer(void) {
    const u16 minFrames = ENEMY_DIR_CHANGE_MIN_FRAMES;
    const u16 maxFrames = ENEMY_DIR_CHANGE_MAX_FRAMES;
    const u16 span = (maxFrames > minFrames) ? (maxFrames - minFrames) : 0;
    const u16 randomOffset = span ? (random() % (span + 1)) : 0;
    return minFrames + randomOffset;
}

static u8 getActiveGiftTargetPos(s16* targetX, s16* targetY) {
    if (targetX == NULL || targetY == NULL) return FALSE;

    for (u8 i = 0; i < NUM_GIFT_DROPS; i++) {
        const GiftDrop* drop = &drops[i];
        if (drop->active) {
            *targetX = drop->x + (GIFT_SIZE / 2);
            *targetY = drop->y + (GIFT_SIZE / 2);
            return TRUE;
        }
        if (drop->pending) {
            *targetX = drop->targetX + (GIFT_SIZE / 2);
            *targetY = drop->targetY + (GIFT_SIZE / 2);
            return TRUE;
        }
    }

    return FALSE;
}

static void onGiftSuccess(void) {
    const u8 previousValue = giftCounterValue;
    if (giftCounterValue < DELIVERY_TARGET) {
        giftCounterValue++;
    }
    deliveriesCompleted = giftCounterValue;
    if (previousValue != giftCounterValue) {
        giftCounter_startBlink(&giftCounterBlink, previousValue, giftCounterValue,
            GIFT_COUNTER_BLINK_INTERVAL_FRAMES);
        giftCounterBlinkStartFrame = frameCounter;
    }
    updateEnemyActivation();
    updateGiftCounter();
}

static void playRandomElfStealSound(void) {
    if (random() & 1) {
        XGM2_playPCM(snd_elfo_volador_robando_1, sizeof(snd_elfo_volador_robando_1), SOUND_PCM_CH_AUTO);
    } else {
        XGM2_playPCM(snd_elfo_volador_robando_2, sizeof(snd_elfo_volador_robando_2), SOUND_PCM_CH_AUTO);
    }
}

static void playGiftDeliveredSound(void) {
    XGM2_playPCM(snd_regalo_recogido, sizeof(snd_regalo_recogido), SOUND_PCM_CH_AUTO);
}

static void playGiftLostSound(void) {
    XGM2_playPCM(snd_regalo_desaparece, sizeof(snd_regalo_desaparece), SOUND_PCM_CH_AUTO);
}

static Chimney* findChimneyAtPoint(s16 x, s16 y) {
    for (u8 i = 0; i < NUM_CHIMNEYS; i++) {
        Chimney* chimney = &chimneys[i];
        if (chimney->prohibited || chimney->state != CHIMNEY_ACTIVE) continue;

        const s16 giftHitX = x + GIFT_HITBOX_OFFSET_X;
        const s16 giftHitY = y + GIFT_HITBOX_OFFSET_Y;
        const s16 chimneyHitX = chimney->x + CHIMNEY_HITBOX_OFFSET_X;
        const s16 chimneyHitY = chimney->y + CHIMNEY_HITBOX_OFFSET_Y;

        if (gameCore_checkCollision(giftHitX, giftHitY, GIFT_HITBOX_WIDTH, GIFT_HITBOX_HEIGHT,
                chimneyHitX, chimneyHitY, CHIMNEY_HITBOX_WIDTH, CHIMNEY_HITBOX_HEIGHT)) {
            return chimney;
        }
    }

    return NULL;
}

static Chimney* findAnyChimneyAtPoint(s16 x, s16 y) {
    for (u8 i = 0; i < NUM_CHIMNEYS; i++) {
        Chimney* chimney = &chimneys[i];
        const s16 giftHitX = x + GIFT_HITBOX_OFFSET_X;
        const s16 giftHitY = y + GIFT_HITBOX_OFFSET_Y;
        const s16 chimneyHitX = chimney->x + CHIMNEY_HITBOX_OFFSET_X;
        const s16 chimneyHitY = chimney->y + CHIMNEY_HITBOX_OFFSET_Y;

        if (gameCore_checkCollision(giftHitX, giftHitY, GIFT_HITBOX_WIDTH, GIFT_HITBOX_HEIGHT,
                chimneyHitX, chimneyHitY, CHIMNEY_HITBOX_WIDTH, CHIMNEY_HITBOX_HEIGHT)) {
            return chimney;
        }
    }

    return NULL;
}

static void resolveGiftDropAtTarget(const GiftDrop* drop) {
    if (drop == NULL) return;

    Chimney* chimney = findChimneyAtPoint(drop->targetX, drop->targetY);
    if (chimney == NULL) {
        Chimney* blocked = findAnyChimneyAtPoint(drop->targetX, drop->targetY);
        if (blocked && blocked->prohibited) {
            // kprintf("[THROW] gift burned at prohibited chimney x=%d y=%d", blocked->x, blocked->y);
            XGM2_playPCM(snd_regalo_quemado, sizeof(snd_regalo_quemado), SOUND_PCM_CH_AUTO);
            /* Entregas en chimeneas prohibidas ya no restan regalos. */
            /* onGiftFailure(); */
        } else {
            // kprintf("[THROW] gift landed no chimney x=%d y=%d", drop->targetX, drop->targetY);
            playGiftLostSound();
        }
        return;
    }

    playGiftDeliveredSound();
    chimney->state = CHIMNEY_COOLDOWN;
    chimney->cooldown = CHIMNEY_RESET_FRAMES;
    chimney->blink = 0;
    if (chimney->usedSprite) {
        SPR_setAnim(chimney->usedSprite, 0);
        SPR_setAnimationLoop(chimney->usedSprite, TRUE);
        SPR_setAutoAnimation(chimney->usedSprite, TRUE);
        SPR_setPosition(chimney->usedSprite, chimney->x, chimney->y);
        SPR_setVisibility(chimney->usedSprite, VISIBLE);
        if (chimney->sprite) {
            SPR_setVisibility(chimney->sprite, HIDDEN);
        }
    }

    onGiftSuccess();
    // kprintf("[THROW] gift delivered at chimney x=%d y=%d deliveries=%u giftsLeft=%u",
    //     chimney->x, chimney->y, deliveriesCompleted, giftCounterValue);
}

static void spawnGiftDrop(void) {
    GiftDrop* drop = NULL;
    for (u8 i = 0; i < NUM_GIFT_DROPS; i++) {
        GiftDrop* candidate = &drops[i];
        if (candidate->pending) {
            drop = candidate;
            break;
        }
    }
    if (drop == NULL) {
        // kprintf("[THROW][WARN] no pending target for spawn");
        return;
    }

    if (drop->sprite == NULL) {
        drop->sprite = SPR_addSpriteSafe(&sprite_regalo, 0, 0,
            TILE_ATTR(PAL_EFFECT, FALSE, FALSE, FALSE));
        if (drop->sprite == NULL) {
            drop->pending = FALSE;
            if (drop->targetSprite) {
                SPR_setVisibility(drop->targetSprite, HIDDEN);
            }
            return;
        }
        SPR_setDepth(drop->sprite, DEPTH_EFFECTS);
        SPR_setAutoAnimation(drop->sprite, FALSE);
        SPR_setAnim(drop->sprite, 0);
    }

    drop->active = TRUE;
    drop->pending = FALSE;
    drop->x = santa.x + SANTA_THROW_OFFSET_X;
    drop->y = santa.y + SANTA_THROW_OFFSET_Y;
    drop->fx = FIX16(drop->x);
    drop->fy = FIX16(drop->y);
    const s16 dx = drop->targetX - drop->x;
    const s16 dy = drop->targetY - drop->y;
    const s16 absDx = (dx < 0) ? -dx : dx;
    const s16 absDy = (dy < 0) ? -dy : dy;
    u16 travelFrames = (absDx > absDy) ? absDx : absDy;
    travelFrames = (travelFrames / GIFT_FLY_SPEED);
    if (travelFrames == 0) {
        travelFrames = 1;
    }
    drop->framesToTarget = travelFrames;
    drop->vx = F16_div(FIX16(dx), FIX16(travelFrames));
    drop->vy = F16_div(FIX16(dy), FIX16(travelFrames));
    SPR_setPosition(drop->sprite, drop->x, drop->y);
    SPR_setVisibility(drop->sprite, VISIBLE);

    // kprintf("[THROW] spawn gift pos=(%d,%d) target=(%d,%d) frames=%u vx=%ld vy=%ld",
    //     drop->x, drop->y, drop->targetX, drop->targetY,
    //     travelFrames, (long)drop->vx, (long)drop->vy);
}

static Chimney* findNearestChimneyInRange(s16 centerX, s16 centerY, u16 radius, s32* outDistanceSq) {
    const s32 radiusSq = (s32)radius * (s32)radius;
    Chimney* nearest = NULL;
    s32 bestDist = radiusSq + 1;

    for (u8 i = 0; i < NUM_CHIMNEYS; i++) {
        Chimney* chimney = &chimneys[i];
        /* Se permite apuntar incluso a chimeneas encendidas; solo se excluyen las usadas en cooldown. */
        if (chimney->state != CHIMNEY_ACTIVE) {
            continue;
        }
        const s16 chimneyCenterX = chimney->x + (CHIMNEY_SIZE / 2);
        const s16 chimneyCenterY = chimney->y + (CHIMNEY_SIZE / 2);
        const s32 dx = (s32)chimneyCenterX - (s32)centerX;
        const s32 dy = (s32)chimneyCenterY - (s32)centerY;
        const s32 distSq = (dx * dx) + (dy * dy);

        if (distSq <= radiusSq && distSq < bestDist) {
            bestDist = distSq;
            nearest = chimney;
        }
    }

    if (outDistanceSq) {
        *outDistanceSq = bestDist;
    }

    return nearest;
}

static void startGiftThrow(void) {
    if (dropCooldown > 0) return;
    if (santaThrowing) return;

    s8 pendingIndex = -1;
    for (u8 i = 0; i < NUM_GIFT_DROPS; i++) {
        if (!drops[i].active && !drops[i].pending) {
            pendingIndex = (s8)i;
            break;
        }
    }
    if (pendingIndex < 0) return;

    GiftDrop* pendingDrop = &drops[(u8)pendingIndex];
    pendingDrop->pending = TRUE;
    pendingDrop->active = FALSE;
    pendingDrop->framesToTarget = 0;
    pendingDrop->vx = FIX16(0);
    pendingDrop->vy = FIX16(0);
    pendingDrop->fx = FIX16(0);
    pendingDrop->fy = FIX16(0);

    const s16 santaThrowX = santa.x + SANTA_THROW_OFFSET_X;
    const s16 santaThrowY = santa.y + SANTA_THROW_OFFSET_Y;
    s32 distanceSq = 0;
    Chimney* nearest = findNearestChimneyInRange(santaThrowX, santaThrowY,
        THROW_TARGET_RADIUS, &distanceSq);

    const s16 fallbackSide = (random() & 1) ? THROW_FALLBACK_OFFSET_X : -THROW_FALLBACK_OFFSET_X;
    s16 targetCenterX = santaThrowX + fallbackSide;
    s16 targetCenterY = santaThrowY;
    if (nearest != NULL) {
        targetCenterX = nearest->x + (CHIMNEY_SIZE / 2);
        targetCenterY = nearest->y + (CHIMNEY_SIZE / 2);
        // kprintf("[THROW] nearest chimney distSq=%ld pos=(%d,%d)", (long)distanceSq, nearest->x, nearest->y);
    } else {
        // kprintf("[THROW] no chimney nearby, fallback target side=%d", (int)fallbackSide);
    }

    pendingDrop->targetX = targetCenterX - (GIFT_SIZE / 2);
    pendingDrop->targetY = targetCenterY - (GIFT_SIZE / 2);
    const s16 markOffset = (GIFT_SIZE - TARGET_MARK_SIZE) / 2;
    const s16 targetMarkX = pendingDrop->targetX + markOffset;
    const s16 targetMarkY = pendingDrop->targetY + markOffset;

    if (pendingDrop->targetSprite == NULL) {
        pendingDrop->targetSprite = SPR_addSpriteSafe(&sprite_marca_x_2, targetMarkX, targetMarkY,
            TILE_ATTR(PAL_PLAYER, FALSE, FALSE, FALSE));
    }
    if (pendingDrop->targetSprite) {
        SPR_setDepth(pendingDrop->targetSprite, DEPTH_MARKERS);
        SPR_setPosition(pendingDrop->targetSprite, targetMarkX, targetMarkY);
        SPR_setVisibility(pendingDrop->targetSprite, VISIBLE);
    }
    // kprintf("[THROW] lock target idx=%d target=(%d,%d)", pendingIndex, pendingDrop->targetX, pendingDrop->targetY);

    santaThrowing = TRUE;
    santaThrowGiftSpawned = FALSE;
    santaReturnToIdle = FALSE;
    dropCooldown = DROP_COOLDOWN_FRAMES;
    // kprintf("[THROW] start cooldown=%d", dropCooldown);

    if (santa.sprite) {
        SPR_setAnim(santa.sprite, 1);
        SPR_setAnimationLoop(santa.sprite, FALSE);
        SPR_setAutoAnimation(santa.sprite, TRUE);
    }
}

static void updateSantaThrowState(void) {
    if (!santaThrowing) return;

    if (santaReturnToIdle || santa.sprite == NULL) {
        santaThrowing = FALSE;
        santaThrowGiftSpawned = FALSE;
        santaReturnToIdle = FALSE;
        if (santa.sprite) {
            SPR_setAnim(santa.sprite, 0);
            SPR_setAnimationLoop(santa.sprite, TRUE);
            SPR_setAutoAnimation(santa.sprite, TRUE);
            // kprintf("[THROW] Santa back to idle");
        }
        if (santa.sprite == NULL) {
            // kprintf("[THROW][WARN] Santa sprite NULL during throw cleanup");
        }
    }
}

static void onSantaFrameChange(Sprite* sprite) {
    if (sprite == NULL || sprite != santa.sprite) return;

    if (santaThrowing) {
        if (!santaThrowGiftSpawned && sprite->frameInd >= SANTA_THROW_SPAWN_FRAME) {
            santaThrowGiftSpawned = TRUE;
            spawnGiftDrop();
            // kprintf("[THROW] Santa frame=%d spawn gift", sprite->frameInd);
        }

        Animation* anim = sprite->animation;
        if ((anim == NULL) || (anim->numFrame == 0) || (sprite->frameInd >= (anim->numFrame - 1))) {
            santaReturnToIdle = TRUE;
            // kprintf("[THROW] animation finished frame=%d", sprite->frameInd);
        }
    }
}

static void checkEnemyCollision(void) {
    /* Colisiones desactivadas temporalmente. */
    return;

    s16 santaHitX = santa.x + ((SANTA_WIDTH - SANTA_HITBOX_WIDTH) / 2);
    s16 santaHitY = santa.y + (SANTA_HEIGHT - SANTA_HITBOX_HEIGHT);

    for (u8 i = 0; i < MAX_ENEMIES; i++) {
        Enemy* enemy = &enemies[i];
        if (!enemy->active) continue;

        const s16 enemyHitX = enemy->x + ENEMY_HITBOX_OFFSET_X;
        const s16 enemyHitY = enemy->y + ENEMY_HITBOX_OFFSET_Y;

        if (gameCore_checkCollision(santaHitX, santaHitY, SANTA_HITBOX_WIDTH, SANTA_HITBOX_HEIGHT,
                enemyHitX, enemyHitY, ENEMY_HITBOX_WIDTH, ENEMY_HITBOX_HEIGHT)) {
            startRecovery();
            break;
        }
    }
}

static void startRecovery(void) {
    recoveringFrames = RECOVERY_FRAMES;
    dropCooldown = DROP_COOLDOWN_FRAMES;
    santaThrowing = FALSE;
    santaThrowGiftSpawned = FALSE;
    santaReturnToIdle = FALSE;
    santa.x = (WORLD_WIDTH - SANTA_WIDTH) / 2;
    santa.y = SANTA_START_Y;
    santa.vx = 0;
    santa.vy = 0;
    if (santa.sprite) {
        SPR_setAnim(santa.sprite, 0);
        SPR_setAnimationLoop(santa.sprite, TRUE);
        SPR_setAutoAnimation(santa.sprite, TRUE);
    }
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
