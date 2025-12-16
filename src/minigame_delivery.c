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
#include "gift_counter.h"

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
#define GIFT_FLY_SPEED 9

#define MARKER_SIZE 32
#define MARKER_SPEED 4
#define MARKER_SCROLL_COMP 2 /* Factor de compensación del scroll al subir (0 = sin ajuste). */
#define MARKER_MIN_Y 0
#define MARKER_MAX_Y 160
#define GIFT_SIZE 32
#define TARGET_MARK_SIZE 16
#define SANTA_THROW_SPAWN_FRAME 4

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
    u8 targetSide;
} GiftDrop;

typedef struct {
    Sprite* sprite;
    s16 x;
    s16 y;
    s8 direction;
} LauncherMarker;

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
static LauncherMarker launcherMarkers[2]; /**< Marcadores laterales para apuntar lanzamientos. */
static Santa santa; /**< Control del sprite principal. */
static u8 santaThrowing; /**< TRUE mientras Santa ejecuta la animación de lanzamiento. */
static u8 santaThrowTarget; /**< Lado objetivo del regalo (0 izquierda, 1 derecha). */
static u8 santaThrowGiftSpawned; /**< TRUE cuando el regalo ya se generó en la animación. */
static u8 santaReturnToIdle; /**< Solicitud de volver a la animación base. */

static Map* mapBackground; /**< Mapa del plano B para el tejado. */
static s16 backgroundOffsetY; /**< Offset vertical del scroll de fondo. */
static fix16 scrollAccumulator; /**< Acumulador de scroll fraccional. */
static fix16 scrollSpeedPerFrame; /**< Velocidad de scroll por frame. */
static SnowEffect snowEffect; /**< Efecto de nieve compartido. */
static GameTimer gameTimer; /**< Temporizador de la fase para derrota. */

static Sprite* giftCounterTop; /**< Contador gráfico fila superior. */
static Sprite* giftCounterBottom; /**< Contador gráfico fila inferior. */
static GiftCounterHUD giftCounterHUD; /**< Configuración del contador de regalos. */
static GiftCounterBlink giftCounterBlink; /**< Parpadeo compartido para el HUD. */

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
static void initLauncherMarkers(void);
static void initGiftCounterSprites(void);
static s16 advanceVerticalScroll(void);
static void applyBackgroundScroll(void);
static void updateChimneys(s16 scrollStep);
static void updateEnemies(s16 scrollStep);
static void updateGiftDrops(s16 scrollStep);
static void updateLauncherMarkers(s16 scrollStep);
static void updateSantaThrowState(void);
static void respawnEnemyFromTop(Enemy* enemy, u8 offsetIndex);
static void resetChimneySpawnCursor(void);
static s16 takeNextChimneySpawnY(void);
static s16 pickChimneyX(void);
static u8 rollChimneyProhibited(void);
static void updateGiftCounter(void);
static void spawnGiftDrop(u8 targetSide);
static void startGiftThrow(u8 targetSide);
static void resolveGiftDropAtTarget(const GiftDrop* drop);
static Chimney* findChimneyAtPoint(s16 x, s16 y);
static s16 abs16(s16 value);
static void checkEnemyCollision(void);
static void startRecovery(void);
static void updateRecovery(void);
static void onSantaFrameChange(Sprite* sprite);

/** @brief Configura recursos, estado inicial de la fase. */
void minigameDelivery_init(void) {
    VDP_setScreenWidth320();
    VDP_setScreenHeight224();

    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);
    VDP_setPlaneSize(64, 64, TRUE);
    SPR_reset();
    kprintf("[SANTA] starting Santa init at pos=(%d,%d)", (WORLD_WIDTH - SANTA_WIDTH) / 2, SANTA_START_Y);

    frameCounter = 0;
    giftsRemaining = DELIVERY_TARGET;
    deliveriesCompleted = 0;
    phaseCompleted = FALSE;
    dropCooldown = 0;
    recoveringFrames = 0;
    previousInput = 0;
    santaThrowing = FALSE;
    santaThrowGiftSpawned = FALSE;
    santaReturnToIdle = FALSE;
    santaThrowTarget = 0;
    backgroundOffsetY = SCROLL_LOOP_PX;
    backgroundOffsetY %= SCROLL_LOOP_PX;
    scrollAccumulator = FIX16(0);
    scrollSpeedPerFrame = SCROLL_SPEED_PER_FRAME;

    
    initBackground();
    initSanta();
    initChimneys();
    initEnemies();
    initGiftDrops();
    initLauncherMarkers();
    initGiftCounterSprites();
    updateGiftCounter();

    gameCore_initTimer(&gameTimer, 120);

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
        kprintf("[SANTA][ERROR] missing sprite at update frame=%u", frameCounter);
        santaMissingLogged = TRUE;
    } else if (santa.sprite && (frameCounter % 120) == 0) {
        kprintf("[SANTA] frame=%u pos=(%d,%d) depth=%d visible_check", frameCounter, santa.x, santa.y, DEPTH_SANTA);
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
            dirX, dirY,
            0, WORLD_WIDTH - SANTA_WIDTH,
            0, SCREEN_HEIGHT - SANTA_HEIGHT,
            frameCounter, &santaInertia);

        if (dropCooldown > 0) {
            dropCooldown--;
        }

        const u8 pressedA = (input & BUTTON_A) && !(previousInput & BUTTON_A);
        const u8 pressedB = (input & BUTTON_B) && !(previousInput & BUTTON_B);

        if (pressedA) {
            startGiftThrow(0);
        } else if (pressedB) {
            startGiftThrow(1);
        }
    }

    s16 scrollStep = advanceVerticalScroll();
    applyBackgroundScroll();
    updateChimneys(scrollStep);
    updateEnemies(scrollStep);
    updateLauncherMarkers(scrollStep);
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
    if (sprite_santa_car_volando.palette) {
        PAL_setPalette(PAL_PLAYER, sprite_santa_car_volando.palette->data, CPU);
    } else if (sprite_santa_car.palette) {
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

    snowEffect_init(&snowEffect, &globalTileIndex, 2, -2);
}

static void initSanta(void) {
    santa.x = (WORLD_WIDTH - SANTA_WIDTH) / 2;
    santa.y = SANTA_START_Y;
    santa.vx = 0;
    santa.vy = 0;

    kprintf("[SANTA] starting Santa init at pos=(%d,%d)", santa.x, santa.y);
    santa.sprite = SPR_addSpriteSafe(&sprite_santa_car_volando, santa.x, santa.y,
        TILE_ATTR(PAL_PLAYER, FALSE, FALSE, FALSE));
    if (santa.sprite) {
        kprintf("[SANTA] sprite created successfully");
        SPR_setDepth(santa.sprite, DEPTH_SANTA);
        SPR_setAutoAnimation(santa.sprite, TRUE);
        SPR_setAnimationLoop(santa.sprite, TRUE);
        SPR_setFrameChangeCallback(santa.sprite, onSantaFrameChange);
        SPR_setVisibility(santa.sprite, VISIBLE);
        kprintf("[SANTA] sprite ok depth=%d pos=(%d,%d)", DEPTH_SANTA, santa.x, santa.y);
    } else {
        kprintf("[SANTA][ERROR] sprite not created");
    }
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
        drops[i].sprite = SPR_addSpriteSafe(&sprite_regalo, 0, 0,
            TILE_ATTR(PAL_EFFECT, FALSE, FALSE, FALSE));
        drops[i].active = FALSE;
        drops[i].targetSide = 0;
        drops[i].targetX = 0;
        drops[i].targetY = 0;
        drops[i].fx = FIX16(0);
        drops[i].fy = FIX16(0);
        drops[i].vx = FIX16(0);
        drops[i].vy = FIX16(0);
        drops[i].framesToTarget = 0;
        drops[i].pending = FALSE;
        drops[i].targetSprite = SPR_addSpriteSafe(&sprite_marca_x, 0, 0,
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

static void initLauncherMarkers(void) {
    memset(launcherMarkers, 0, sizeof(launcherMarkers));

    launcherMarkers[0].x = CHIMNEY_X_LEFT;
    launcherMarkers[0].y = MARKER_MIN_Y;
    launcherMarkers[0].direction = MARKER_SPEED;

    launcherMarkers[1].x = CHIMNEY_X_RIGHT;
    launcherMarkers[1].y = MARKER_MAX_Y;
    launcherMarkers[1].direction = -MARKER_SPEED;

    for (u8 i = 0; i < 2; i++) {
        LauncherMarker* marker = &launcherMarkers[i];
        marker->sprite = SPR_addSpriteSafe(&sprite_marca_lanzador, marker->x, marker->y,
            TILE_ATTR(PAL_PLAYER, FALSE, FALSE, FALSE));
        if (marker->sprite) {
            SPR_setDepth(marker->sprite, DEPTH_MARKERS);
            SPR_setAutoAnimation(marker->sprite, FALSE);
            SPR_setVisibility(marker->sprite, VISIBLE);
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

    giftCounter_initHUD(&giftCounterHUD, giftCounterTop, giftCounterBottom,
        baseX, baseY, -16, 12, DEPTH_HUD + 1, DEPTH_HUD, 5, DELIVERY_TARGET);
    giftCounter_stopBlink(&giftCounterBlink);
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

static void updateLauncherMarkers(s16 scrollStep) {
    for (u8 i = 0; i < 2; i++) {
        LauncherMarker* marker = &launcherMarkers[i];
        s16 delta = marker->direction;
        if (delta < 0 && scrollStep > 0 && MARKER_SCROLL_COMP > 0) {
            delta -= (scrollStep * MARKER_SCROLL_COMP); /* Ajustable para igualar tiempos subida/bajada. */
        }

        marker->y += delta;

        if (marker->y <= MARKER_MIN_Y) {
            marker->y = MARKER_MIN_Y;
            marker->direction = MARKER_SPEED;
            kprintf("[MARKER %d] bounce TOP y=%d scroll=%d time=%d", i, marker->y, scrollStep, gameTimer.elapsed);
        } else if (marker->y >= MARKER_MAX_Y) {
            marker->y = MARKER_MAX_Y;
            marker->direction = -MARKER_SPEED;
            kprintf("[MARKER %d] bounce BOTTOM y=%d scroll=%d time=%d", i, marker->y, scrollStep, gameTimer.elapsed);
        }

        if (marker->sprite) {
            SPR_setPosition(marker->sprite, marker->x, marker->y);
            SPR_setDepth(marker->sprite, DEPTH_MARKERS);
            SPR_setVisibility(marker->sprite, VISIBLE);
        }
    }
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
        }

        if (drop->targetSprite) {
            const s16 markOffset = (GIFT_SIZE - TARGET_MARK_SIZE) / 2;
            const s16 targetMarkX = drop->targetX + markOffset;
            const s16 targetMarkY = drop->targetY + markOffset;
            SPR_setDepth(drop->targetSprite, DEPTH_MARKERS);
            SPR_setPosition(drop->targetSprite, targetMarkX, targetMarkY);
            SPR_setVisibility(drop->targetSprite, VISIBLE);
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
                drop->active = FALSE;
                drop->pending = FALSE;
                SPR_setVisibility(drop->sprite, HIDDEN);
                if (drop->targetSprite) {
                    SPR_setVisibility(drop->targetSprite, HIDDEN);
                }
            }
        }
    }
}

static s16 abs16(s16 value) {
    return (value < 0) ? -value : value;
}

static void updateGiftCounter(void) {
    const u16 displayValue = giftCounter_getDisplayValue(&giftCounterBlink,
        giftsRemaining, frameCounter);
    giftCounter_render(&giftCounterHUD, displayValue);
}

static Chimney* findChimneyAtPoint(s16 x, s16 y) {
    for (u8 i = 0; i < NUM_CHIMNEYS; i++) {
        Chimney* chimney = &chimneys[i];
        if (chimney->prohibited || chimney->state != CHIMNEY_ACTIVE) continue;

        if (gameCore_checkCollision(x, y, GIFT_SIZE, GIFT_SIZE,
                chimney->x, chimney->y, CHIMNEY_SIZE, CHIMNEY_SIZE)) {
            return chimney;
        }
    }

    return NULL;
}

static void resolveGiftDropAtTarget(const GiftDrop* drop) {
    if (drop == NULL) return;

    Chimney* chimney = findChimneyAtPoint(drop->targetX, drop->targetY);
    if (chimney == NULL) {
        kprintf("[THROW] gift landed no chimney x=%d y=%d", drop->targetX, drop->targetY);
        return;
    }

    chimney->state = CHIMNEY_COOLDOWN;
    chimney->cooldown = CHIMNEY_RESET_FRAMES;
    chimney->blink = 0;

    if (deliveriesCompleted < DELIVERY_TARGET) {
        deliveriesCompleted++;
    }
    if (giftsRemaining > 0) {
        giftsRemaining--;
    }
    kprintf("[THROW] gift delivered at chimney x=%d y=%d deliveries=%u giftsLeft=%u",
        chimney->x, chimney->y, deliveriesCompleted, giftsRemaining);
}

static void spawnGiftDrop(u8 targetSide) {
    GiftDrop* drop = NULL;
    u8 dropIndex = 0;
    for (u8 i = 0; i < NUM_GIFT_DROPS; i++) {
        GiftDrop* candidate = &drops[i];
        if (candidate->pending && candidate->targetSide == targetSide) {
            drop = candidate;
            dropIndex = i;
            break;
        }
    }
    if (drop == NULL) {
        kprintf("[THROW][WARN] no pending target for side=%u", targetSide);
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
    drop->x = santa.x + 38;
    drop->y = santa.y + 110;
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

    kprintf("[THROW] spawn gift idx=%u targetSide=%u pos=(%d,%d) target=(%d,%d) frames=%u vx=%ld vy=%ld",
        dropIndex, targetSide, drop->x, drop->y, drop->targetX, drop->targetY,
        travelFrames, (long)drop->vx, (long)drop->vy);
}

static void startGiftThrow(u8 targetSide) {
    if (giftsRemaining == 0) return;
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

    GiftDrop* pendingDrop = &drops[pendingIndex];
    pendingDrop->pending = TRUE;
    pendingDrop->active = FALSE;
    pendingDrop->targetSide = targetSide;
    pendingDrop->framesToTarget = 0;
    pendingDrop->vx = FIX16(0);
    pendingDrop->vy = FIX16(0);
    pendingDrop->fx = FIX16(0);
    pendingDrop->fy = FIX16(0);

    const LauncherMarker* marker = &launcherMarkers[(targetSide != 0) ? 1 : 0];
    const s16 markerCenterX = marker->x + (MARKER_SIZE / 2);
    const s16 markerCenterY = marker->y + (MARKER_SIZE / 2);
    pendingDrop->targetX = markerCenterX - (GIFT_SIZE / 2);
    pendingDrop->targetY = markerCenterY - (GIFT_SIZE / 2);
    const s16 markOffset = (GIFT_SIZE - TARGET_MARK_SIZE) / 2;
    const s16 targetMarkX = pendingDrop->targetX + markOffset;
    const s16 targetMarkY = pendingDrop->targetY + markOffset;

    if (pendingDrop->targetSprite == NULL) {
        pendingDrop->targetSprite = SPR_addSpriteSafe(&sprite_marca_x, targetMarkX, targetMarkY,
            TILE_ATTR(PAL_PLAYER, FALSE, FALSE, FALSE));
    }
    if (pendingDrop->targetSprite) {
        SPR_setDepth(pendingDrop->targetSprite, DEPTH_MARKERS);
        SPR_setPosition(pendingDrop->targetSprite, targetMarkX, targetMarkY);
        SPR_setVisibility(pendingDrop->targetSprite, VISIBLE);
    }
    kprintf("[THROW] lock target idx=%d side=%u target=(%d,%d)", pendingIndex, targetSide, pendingDrop->targetX, pendingDrop->targetY);

    santaThrowing = TRUE;
    santaThrowTarget = targetSide;
    santaThrowGiftSpawned = FALSE;
    santaReturnToIdle = FALSE;
    dropCooldown = DROP_COOLDOWN_FRAMES;
    kprintf("[THROW] start side=%u giftsRemaining=%u cooldown=%d", santaThrowTarget, giftsRemaining, dropCooldown);

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
            kprintf("[THROW] Santa back to idle");
        }
        if (santa.sprite == NULL) {
            kprintf("[THROW][WARN] Santa sprite NULL during throw cleanup");
        }
    }
}

static void onSantaFrameChange(Sprite* sprite) {
    if (sprite == NULL || sprite != santa.sprite) return;

    if (santaThrowing) {
        if (!santaThrowGiftSpawned && sprite->frameInd >= SANTA_THROW_SPAWN_FRAME) {
            santaThrowGiftSpawned = TRUE;
            spawnGiftDrop(santaThrowTarget);
            kprintf("[THROW] Santa frame=%d spawn gift side=%u", sprite->frameInd, santaThrowTarget);
        }

        Animation* anim = sprite->animation;
        if ((anim == NULL) || (anim->numFrame == 0) || (sprite->frameInd >= (anim->numFrame - 1))) {
            santaReturnToIdle = TRUE;
            kprintf("[THROW] animation finished frame=%d", sprite->frameInd);
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
    santaThrowing = FALSE;
    santaThrowGiftSpawned = FALSE;
    santaReturnToIdle = FALSE;
    santaThrowTarget = 0;
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
