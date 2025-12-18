/**
 * ═════════════════════════════════════════════════════════════════════════════
 * ARCHIVO: src/minigame_pickup.c
 *
 * Fase 1: Recogida - Paseo cenital con pista de nieve y scroll vertical.
 * Estado actual: mecanicas acotadas; falta disparo especial definitivo y sprite de regalo lateral.
 *
 * Recursos y paletas usados en la fase:
 * - Fondos: `resources_bg.h` aporta el `mapTrack` y su paleta de nieve aplicada
 *   al plano B con el índice `globalTileIndex` como base de carga.
 * - Sprites: definidos en `resources_sprites.h` para Santa, árboles, elfos y
 *   regalos. Cada sprite usa su propia paleta incluida en el mismo fichero.
 * - Efectos de sonido: `resources_sfx.h` (aterrizaje de regalos, colisiones y
 *   enemigos) sin impacto en paletas.
 * - Efectos visuales: `snow_effect.h` genera partículas independientes que
 *   reutilizan la paleta del fondo nevado.
 * - Música: gestionada por `audio_manager.h` a partir de `resources_music.h`.
 * ═════════════════════════════════════════════════════════════════════════════
 */

/** @file minigame_pickup.c
 *  @brief Fase 1: Recogida con scroll vertical, nieve y gestión de regalos.
 */
#include "minigame_pickup.h"
#include "audio_manager.h"
#include "resources_bg.h"
#include "resources_sprites.h"
#include "resources_sfx.h"
#include "snow_effect.h"
#include "gift_counter.h"

static void traceFunc(const char *funcName);
#define TRACE_FUNC() traceFunc(__func__)

#define NUM_TREES 2              /* Árboles simultáneos en pista (máximo). */
#define NUM_ELVES  4             /* Elfos que aparecen en oleadas. */
#define NUM_ENEMIES 3            /* Enemigos concurrentes. */
#define GIFTS_FOR_SPECIAL 3      /* Regalos necesarios para cargar especial. */
#define GIFTS_FOR_SECOND_TREE 7  /* Regalos necesarios para activar el segundo árbol. */
#define TARGET_GIFTS 10          /* Objetivo de regalos para cambiar de fase. */
#define SCROLL_SPEED 1           /* Velocidad base de scroll vertical. */
#define FORBIDDEN_PERCENT 10     /* Margen lateral no jugable (porcentaje). */
#define TRACK_LOOP_PX 512        /* Altura del bucle de pista. */

#define ENEMY_LATERAL_DELAY 10   /* Retardo entre ajustes laterales del enemigo. */
#define ENEMY_LATERAL_SPEED 1    /* Velocidad lateral del enemigo. */
#define ENEMY_ESCAPE_SPEED 3     /* Velocidad de escape tras robar. */

#define TREE_SIZE 64             /* Tamaño de sprite del árbol. */
#define TREE_HITBOX_OFFSET_X 8     /* Margen izquierdo desde el origen del sprite */
#define TREE_HITBOX_OFFSET_Y 52    /* Margen superior desde el origen del sprite */
#define TREE_HITBOX_WIDTH 45       /* Ancho de la zona colisionable (53 - 8) */
#define TREE_HITBOX_HEIGHT 6       /* Alto de la zona colisionable (58 - 52) */
#define ENEMY_SIZE 32            /* Tamaño del enemigo lateral. */
#define ENEMY_HITBOX_HEIGHT 10   /* Altura útil del hitbox de enemigo. */
#define ELF_SIZE 32              /* Tamaño del elfo lateral. */
#define ELF_MARK_SIZE 16         /* Tamaño del icono indicador de elfo. */
#define ELF_MARK_VISIBLE_MIN_Y 150       /* y inferior mínima para mostrar X (arriba) */
#define ELF_MARK_VISIBLE_MAX_Y 210      /* y inferior máxima para mostrar X (abajo) */
#define ELF_MARK_SCREEN_MARGIN_PERCENT 20 /* Margen lateral para marcas en pantalla. */
#define ELF_SHADOW_MIN_DEPTH SPR_MAX_DEPTH /* Profundidad mínima para sombras. */
#define ELF_RESPAWN_DELAY_MIN_FRAMES 60   /* 1s a 60fps */
#define ELF_RESPAWN_DELAY_MAX_FRAMES 300  /* 5s a 60fps */
#define GIFT_SIZE 32             /* Tamaño del sprite de regalo. */
#define GIFT_ARC_HEIGHT 50       /* Altura máxima de la parábola del regalo. */
#define GIFT_COUNTER_ROW_SIZE 5  /* Regalos por fila en HUD. */
#define GIFT_COUNTER_SPRITE_WIDTH 96 /* Ancho del sprite contador. */
#define GIFT_COUNTER_SPRITE_HEIGHT 24 /* Alto del sprite contador. */
#define GIFT_COUNTER_ROW_OFFSET_Y 12 /* Separación vertical entre filas de contador. */
#define GIFT_COUNTER_SECOND_ROW_OFFSET_X 6 /* Desfase X de la segunda fila HUD. */
#define GIFT_COUNTER_MAX 10      /* Límite de regalos mostrados. */
#define GIFT_LOSS_MAX_OFFSET 2   /* Hasta cuanto puede bajar el contador respecto al máximo. */
#define MUSIC_START_DELAY_FRAMES 40 /* Frames de retraso antes de arrancar música. */
#define MUSIC_FM_VOLUME 70          /* Volumen FM para la pista de fase 1. */
#define MUSIC_PSG_VOLUME 100        /* Volumen PSG para la pista de fase 1. */
#define SANTA_WIDTH 80              /* Ancho del sprite de Santa. */
#define SANTA_HEIGHT 128            /* Alto del sprite de Santa. */
#define SANTA_HITBOX_PADDING 30   /* píxeles que no colisionan a cada lado */
#define SANTA_COLECT_EXTRA_MARGIN 10 /* Margen extra para recoger regalos */
#define SANTA_HITBOX_WIDTH (SANTA_WIDTH - 2 * SANTA_HITBOX_PADDING) /* Ancho hitbox. */
#define SANTA_VERTICAL_SPEED 2      /* Velocidad vertical base de Santa. */
#define TRACK_HEIGHT_PX 512         /* Altura total del trazado. */
#define SCROLL_SPEED_MIN FIX16(0.5) /* Velocidad mínima del scroll. */
#define SCROLL_SPEED_MAX FIX16(1.5) /* Velocidad máxima del scroll. */
#define HUD_MARGIN_PX 5             /* Margen de HUD en píxeles. */
#define DEPTH_HUD SPR_MIN_DEPTH     /* HUD siempre por delante del resto. */
#define DEPTH_SANTA (SPR_MIN_DEPTH + 24)   /* Santa delante de actores pero tras el HUD. */
#define DEPTH_ACTORS_START (DEPTH_SANTA + 2) /* Profundidad inicial de actores. */
#define TREE_COLLISION_BLINK_FRAMES 120 /* Duración del parpadeo tras choque. */
#define TREE_COLLISION_BLINK_INTERVAL_FRAMES 6 /* Intervalo de parpadeo. */
#define GIFT_COUNTER_BLINK_INTERVAL_FRAMES 3 /* Intervalo de parpadeo del HUD. */

/** @brief Actor básico con sprite y coordenadas. */
typedef struct {
    Sprite* sprite;
    s16 x, y;
    u8 active;
} SimpleActor;

/** @brief Datos principales del trineo de Santa. */
typedef struct {
    Sprite* sprite;
    s16 x, y;
    s8 vx;
    s8 vy;
    u8 specialReady;
} Santa;

static Santa santa; /**< Estado del sprite controlable de Santa. */
static SimpleActor trees[NUM_TREES]; /**< Obstáculos de árboles en pista. */
static SimpleActor elves[NUM_ELVES]; /**< Peatones que reciben regalos. */
static SimpleActor enemies[NUM_ENEMIES]; /**< Enemigos que roban regalos. */
static s16 elfSpawnY[NUM_ELVES]; /**< Puntos de aparición vertical de elfos. */
static u8 elfMarkShown[NUM_ELVES]; /**< Indicador de X mostrado sobre cada elfo. */
static Sprite* elfMarkSprites[NUM_ELVES]; /**< Sprites de las marcas flotantes. */
static s16 elfMarkPosX[NUM_ELVES]; /**< Posición X del indicador de elfo. */
static s16 elfMarkPosY[NUM_ELVES]; /**< Posición Y del indicador de elfo. */
static Sprite* elfShadowSprites[NUM_ELVES]; /**< Sprites de sombra de los elfos. */
static s16 elfShadowStartX[NUM_ELVES]; /**< Punto inicial X para desplazar sombras. */
static s16 elfShadowStartY[NUM_ELVES]; /**< Punto inicial Y para desplazar sombras. */
static u8 elfShadowActive[NUM_ELVES]; /**< Estado visible/activo de cada sombra. */
static s16 elfShadowPosX[NUM_ELVES]; /**< Posición X actual de la sombra. */
static s16 elfShadowPosY[NUM_ELVES]; /**< Posición Y actual de la sombra. */
static Sprite* elfGiftSprites[NUM_ELVES]; /**< Sprites de regalo lanzado a elfos. */
static u8 elfGiftActive[NUM_ELVES]; /**< Si el regalo está volando hacia el elfo. */
static u8 elfGiftHasLanded[NUM_ELVES]; /**< Si el regalo ya aterrizó junto al elfo. */
static s16 elfGiftPosX[NUM_ELVES]; /**< Posición X del regalo en vuelo. */
static s16 elfGiftPosY[NUM_ELVES]; /**< Posición Y del regalo en vuelo. */
static u16 elfRespawnTimer[NUM_ELVES]; /**< Temporizador de reaparición por elfo. */
static u8 elfSide[NUM_ELVES]; /**< Lado del camino donde aparece cada elfo. */
static Map *mapTrack; /**< Mapa de la pista nevosa en BG. */
static s16 trackOffsetY; /**< Desfase vertical acumulado del scroll. */
static fix16 scrollSpeedPerFrame; /**< Velocidad actual de scroll en fix16. */
static fix16 scrollAccumulator; /**< Acumulador de scroll fraccional. */
static u16 giftsCollected; /**< Regalos entregados hasta ahora. */
static u16 maxGiftsCollected; /**< Máximo histórico para estadísticas. */
static u16 giftsCharge; /**< Carga para activar disparo especial. */
static Sprite* giftCounterSpriteFirstRow; /**< Contador HUD fila superior. */
static Sprite* giftCounterSpriteSecondRow; /**< Contador HUD fila inferior. */
static GiftCounterHUD giftCounterHUD; /**< Configuración del contador de HUD. */
static GiftCounterBlink giftCounterBlink; /**< Parpadeo durante pérdidas de regalos. */
static u16 frameCounter; /**< Contador general de frames. */
static u8 phaseChangeRequested; /**< Marca cuando se debe pasar de fase. */
static u8 musicStarted; /**< Indica si la música ya se lanzó. */
static u16 musicStartDelayFrames; /**< Retraso hasta reproducir música. */
static u8 santaAnimationPaused; /**< Pausa animación de Santa tras choque. */

static s16 leftLimit; /**< Límite izquierdo de la pista jugable. */
static s16 rightLimit; /**< Límite derecho de la pista jugable. */
static s16 playableWidth; /**< Ancho utilizable entre límites. */
static s16 santaMinY; /**< Límite superior para Santa. */
static s16 santaMaxY; /**< Límite inferior para Santa. */
static s16 santaStartX; /**< Coordenada X inicial de Santa. */
static s16 santaStartY; /**< Coordenada Y inicial de Santa. */
static GameInertia santaInertia; /**< Parámetros de inercia de movimiento. */
static SnowEffect snowEffect; /**< Sistema de partículas de nieve. */
static const char* lastTraceFunc = NULL; /**< Última función trazada. */
static u8 recoveringFromTree; /**< Flag de recuperación tras chocar con árbol. */
static u16 treeCollisionBlinkFrames; /**< Duración del parpadeo de colisión. */
static u8 treeCollisionVisible; /**< Controla la visibilidad durante parpadeo. */
static SimpleActor* collidedTree; /**< Referencia al árbol con el que chocó Santa. */
static u8 enemyStealActive; /**< Secuencia de robo de regalo en curso. */
static u8 enemyStealIndex; /**< Índice del enemigo que roba. */
static s16 enemyEscapeTargetX; /**< Destino X del enemigo al huir. */
static s16 enemyEscapeTargetY; /**< Destino Y del enemigo al huir. */
static u8 activeEnemyCount;  /**< Número actual de enemigos activos (empieza en 1). */
static u8 secondTreeSpawned; /**< TRUE cuando el segundo árbol ya está activo. */

/**
 * @brief Traza cambios de función para depuración ligera.
 * @param funcName Nombre de la función llamada.
 */
static void traceFunc(const char *funcName) {
    #if DEBUG_MODE
    if (funcName == NULL) return;
    if (lastTraceFunc != funcName) {
        lastTraceFunc = funcName;
        kprintf("[TRACE] %s", funcName);
    }
    #endif
}

static void collectGift(void);
static void updateGiftCounter(void);
static void beginTreeCollision(SimpleActor *tree);
static void updateTreeCollisionRecovery(void);
static void endTreeCollisionRecovery(void);
static void selectEnemyEscapeTarget(SimpleActor *enemy, s16 *targetX, s16 *targetY);
static void beginEnemyStealSequence(u8 enemyIndex);
static void updateEnemyStealSequence(void);
static void endEnemyStealSequence(void);
static void startMusicAfterHoHoHo(void);
static void clearEnemiesOnTreeCollision(void);
static void clearElvesOnTreeCollision(void);
static void clearOtherTreesOnCollision(const SimpleActor *treeToKeep);
static void pauseSantaAnimation(void);
static void resumeSantaAnimation(void);
static u16 giftsLossFloor(void);
static u8 applyGiftLoss(u16 amount);

/**
 * @brief Comprueba un rango horizontal y registra errores si es inválido.
 * @param minX Límite izquierdo permitido.
 * @param maxX Límite derecho permitido.
 * @param context Texto para identificar el origen del chequeo.
 * @return TRUE si el rango es válido.
 */
static u8 validateHorizontalRange(s16 minX, s16 maxX, const char* context) {
    TRACE_FUNC();
    if (maxX <= minX) {
        kprintf("[%s] Rango X invalido (min=%d, max=%d)", context, minX, maxX);
        return FALSE;
    }
    return TRUE;
}

/**
 * @brief Desactiva un actor y oculta su sprite reportando el contexto.
 * @param actor Actor a desactivar.
 * @param context Etiqueta para trazas.
 */
static void markActorInactive(SimpleActor *actor, const char* context) {
    TRACE_FUNC();
    actor->active = FALSE;
    kprintf("[%s] Actor desactivado por error de inicializacion", context);
    if (actor->sprite != NULL) {
        SPR_setVisibility(actor->sprite, HIDDEN);
    }
}

/**
 * @brief Sitúa un actor en una posición aleatoria dentro de un rango.
 * @param actor Actor a posicionar.
 * @param minX Límite izquierdo.
 * @param maxX Límite derecho.
 * @param minY Límite superior.
 * @param maxY Límite inferior.
 */
static void placeActor(SimpleActor *actor, s16 minX, s16 maxX, s16 minY, s16 maxY) {
    TRACE_FUNC();
    actor->x = minX + (random() % (maxX - minX));
    actor->y = -((random() % (maxY - minY)) + minY);
    actor->active = TRUE;
}

/**
 * @brief Devuelve un retardo aleatorio entre dos límites de frames.
 */
static u16 randomFrameDelay(u16 minFrames, u16 maxFrames) {
    TRACE_FUNC();
    if (maxFrames <= minFrames) return minFrames;
    return minFrames + (random() % (maxFrames - minFrames + 1));
}

/**
 * @brief Oculta la marca superior del elfo indicado.
 * @param index Índice del elfo en el array.
 */
static void hideElfMark(u8 index) {
    if (index >= NUM_ELVES) return;
    TRACE_FUNC();
    elfMarkShown[index] = FALSE;
    if (elfMarkSprites[index]) {
        SPR_setVisibility(elfMarkSprites[index], HIDDEN);
    }
}

/**
 * @brief Calcula una posición aleatoria dejando un margen porcentual.
 */
static s16 randomPositionWithMargin(s16 totalSize, s16 elementSize, u8 marginPercent) {
    TRACE_FUNC();
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

/**
 * @brief Muestra la marca superior del elfo indicado.
 * @param index Índice del elfo en el array.
 */
static void showElfMark(u8 index) {
    if (index >= NUM_ELVES) return;
    TRACE_FUNC();
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
    }
}

/** @brief Posiciona un árbol aleatorio en el escenario. */
static void spawnTree(SimpleActor *tree) {
    TRACE_FUNC();
    /* No permitir spawn del segundo árbol hasta que esté desbloqueado. */
    if ((tree == &trees[1]) && !secondTreeSpawned) {
        tree->active = FALSE;
        if (tree->sprite) {
            SPR_setVisibility(tree->sprite, HIDDEN);
        }
        return;
    }
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
    SPR_setVisibility(tree->sprite, VISIBLE);
}

/**
 * @brief Spawnea un elfo que lanza regalo desde un lado concreto.
 * @param elf Actor a inicializar.
 * @param side 0 izquierda, 1 derecha.
 * @param index Índice del elfo para tablas auxiliares.
 */
static void spawnElf(SimpleActor *elf, u8 side, u8 index) {
    if (index >= NUM_ELVES) return;
    TRACE_FUNC();
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
    SPR_setAnim(elf->sprite, 0);
    SPR_setAutoAnimation(elf->sprite, TRUE);
    SPR_setPosition(elf->sprite, elf->x, elf->y);
    SPR_setVisibility(elf->sprite, VISIBLE);
}

/** @brief Crea un enemigo lateral que intentará robar el regalo. */
static void spawnEnemy(SimpleActor *enemy) {
    TRACE_FUNC();
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
    SPR_setAnim(enemy->sprite, 0);
    SPR_setAutoAnimation(enemy->sprite, TRUE);
    SPR_setVisibility(enemy->sprite, VISIBLE);
}

/** @brief Oculta la sombra asociada al elfo indicado. */
static void hideElfShadow(u8 index) {
    if (index >= NUM_ELVES) return;
    TRACE_FUNC();
    elfShadowActive[index] = FALSE;
    if (elfShadowSprites[index]) {
        SPR_setVisibility(elfShadowSprites[index], HIDDEN);
    }
    elfShadowPosX[index] = elfShadowStartX[index];
    elfShadowPosY[index] = elfShadowStartY[index];
}

/**
 * @brief Activa y posiciona la sombra de un elfo.
 * @param index Índice del elfo.
 * @param startX Posición X inicial.
 * @param startY Posición Y inicial.
 */
static void showElfShadow(u8 index, s16 startX, s16 startY) {
    if (index >= NUM_ELVES) return;
    TRACE_FUNC();
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

/**
 * @brief Oculta el regalo lanzado por el elfo.
 * @param index Índice del elfo.
 * @param playDisappearSfx TRUE para reproducir sonido de desaparición.
 */
static void hideElfGift(u8 index, u8 playDisappearSfx) {
    if (index >= NUM_ELVES) return;
    TRACE_FUNC();
    if (playDisappearSfx && elfGiftActive[index]) {
        XGM2_playPCM(snd_regalo_desaparece, sizeof(snd_regalo_desaparece), SOUND_PCM_CH_AUTO);
    }
    elfGiftActive[index] = FALSE;
    elfGiftHasLanded[index] = FALSE;
    if (elfGiftSprites[index]) {
        SPR_setVisibility(elfGiftSprites[index], HIDDEN);
    }
}

/**
 * @brief Activa el regalo lanzado por el elfo y lo posiciona.
 * @param index Índice del elfo.
 * @param startX Posición inicial X.
 * @param startY Posición inicial Y.
 */
static void showElfGift(u8 index, s16 startX, s16 startY) {
    if (index >= NUM_ELVES) return;
    TRACE_FUNC();
    elfGiftActive[index] = TRUE;
    elfGiftHasLanded[index] = FALSE;
    elfGiftPosX[index] = startX;
    elfGiftPosY[index] = startY;
    if (elfGiftSprites[index] == NULL) {
        elfGiftSprites[index] = SPR_addSpriteSafe(&sprite_regalo, startX, startY,
            TILE_ATTR(PAL_EFFECT, FALSE, FALSE, FALSE));
    }
    if (elfGiftSprites[index]) {
        SPR_setVisibility(elfGiftSprites[index], VISIBLE);
        SPR_setPosition(elfGiftSprites[index], startX, startY);
        // Reproduce aleatoriamente snd_regalo_disparado1, 2 o 3
        switch (random() % 3) {
            case 0:
                XGM2_playPCM(snd_regalo_disparado1, sizeof(snd_regalo_disparado1), SOUND_PCM_CH_AUTO);
                break;
            case 1:
                XGM2_playPCM(snd_regalo_disparado2, sizeof(snd_regalo_disparado2), SOUND_PCM_CH_AUTO);
                break;
            case 2:
                XGM2_playPCM(snd_regalo_disparado3, sizeof(snd_regalo_disparado3), SOUND_PCM_CH_AUTO);
                break;
        }
    } else {
        elfGiftActive[index] = FALSE;
    }
    /* El elfo cambia a la animacion 1 sin loop mientras lanza */
    if (elves[index].sprite) {
        SPR_setAnim(elves[index].sprite, 1);
        SPR_setAnimationLoop(elves[index].sprite, FALSE);
    }
}

/**
 * @brief Actualiza la posición del regalo siguiendo una parábola.
 * @param index Índice del elfo asociado.
 * @param progress Progreso normalizado del lanzamiento (0-1).
 */
static void updateElfGift(u8 index, fix16 progress) {
    if (index >= NUM_ELVES) return;
    TRACE_FUNC();
    if (!elfGiftActive[index] || elfGiftSprites[index] == NULL) {
        return;
    }

    if (progress <= FIX16(0)) {
        SPR_setPosition(elfGiftSprites[index], elfShadowStartX[index], elfShadowStartY[index]);
        elfGiftPosX[index] = elfShadowStartX[index];
        elfGiftPosY[index] = elfShadowStartY[index];
        elfGiftHasLanded[index] = FALSE;
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
    elfGiftPosX[index] = posX;
    elfGiftPosY[index] = posY;
    elfGiftHasLanded[index] = (progress >= FIX16(1));
    if (elfGiftHasLanded[index]) {
        kprintf("[DEBUG GIFT] landed idx=%d pos=(%d,%d)", index, posX, posY);
    }
}

/**
 * @brief Programa el respawn de un elfo tras completar su acción.
 * @param index Índice del elfo.
 * @param side Lado desde el que reaparecerá.
 */
static void scheduleElfRespawn(u8 index, u8 side) {
    if (index >= NUM_ELVES) return;
    TRACE_FUNC();
    elfSide[index] = side;
    elfRespawnTimer[index] = randomFrameDelay(ELF_RESPAWN_DELAY_MIN_FRAMES, ELF_RESPAWN_DELAY_MAX_FRAMES);
    elves[index].active = FALSE;
    if (elves[index].sprite) {
        SPR_setAnim(elves[index].sprite, 0);
        SPR_setAutoAnimation(elves[index].sprite, TRUE);
        SPR_setVisibility(elves[index].sprite, HIDDEN);
    }
    hideElfMark(index);
    hideElfShadow(index);
    hideElfGift(index, TRUE);
    kprintf("[ELF %d] Respawn en %u frames (side=%d)", index, elfRespawnTimer[index], side);
}

/**
 * @brief Interpola la posición de la sombra del elfo durante el salto.
 * @param index Índice del elfo.
 * @param progress Progreso normalizado del salto.
 */
static void updateElfShadow(u8 index, fix16 progress) {
    if (index >= NUM_ELVES) return;
    TRACE_FUNC();
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

/**
 * @brief Controla cuándo mostrar la marca X del elfo para facilitar su captura.
 * @param index Índice del elfo.
 * @param santaHitX Posición X de la hitbox de Santa.
 * @param santaHitY Posición Y de la hitbox de Santa.
 * @param santaHitW Anchura de la hitbox.
 * @param santaHitH Altura de la hitbox.
 */
static void updateElfMark(u8 index, s16 santaHitX, s16 santaHitY, s16 santaHitW, s16 santaHitH) {
    if (index >= NUM_ELVES) return;
    TRACE_FUNC();
    const s16 elfBottom = elves[index].y + ELF_SIZE;

    if (!elves[index].active) {
        hideElfMark(index);
        hideElfShadow(index);
        hideElfGift(index, FALSE);
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
        hideElfGift(index, FALSE);
        return;
    }

    if (aboveRange) {
        hideElfMark(index);
        hideElfShadow(index);
        hideElfGift(index, TRUE);
        return;
    }

    /* Muestra solo dentro del rango visible (una vez por aparición) */
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

    if (elfGiftActive[index]  && progress >= FIX16(0.9)) { // Empieza a checkear desde el 90% de caída
        if (gameCore_checkCollision(santaHitX-SANTA_COLECT_EXTRA_MARGIN, santaHitY-SANTA_COLECT_EXTRA_MARGIN, santaHitW+SANTA_COLECT_EXTRA_MARGIN*2, santaHitH+SANTA_COLECT_EXTRA_MARGIN*2, // Hitbox de Santa con margen extra
                elfGiftPosX[index], elfGiftPosY[index], GIFT_SIZE, GIFT_SIZE)) {
            kprintf("[DEBUG GIFT] collect landed idx=%d giftPos=(%d,%d) santaHit=(%d,%d,%d,%d)", index,
                elfGiftPosX[index], elfGiftPosY[index], santaHitX, santaHitY, santaHitW, santaHitH);
            collectGift();
            hideElfMark(index);
            hideElfShadow(index);
            hideElfGift(index, FALSE);
            scheduleElfRespawn(index, elfSide[index]);
        }
    }
}

/** @brief Reinicia el ataque especial si ya estaba cargado. */
static void resetSpecialIfReady(void) {
    TRACE_FUNC();
    if (giftsCharge >= GIFTS_FOR_SPECIAL) {
        santa.specialReady = TRUE;
        /* Animacion alternativa desactivada para evitar errores si no existe */
        SPR_setAnim(santa.sprite, 0);
    }
}

/** @brief Marca que la fase ha cumplido objetivo y debe finalizar. */
static void requestPhaseChange(void) {
    TRACE_FUNC();
    /* TODO: implementar cambio de fase */
    kprintf("[PHASE] Cambio de fase solicitado (pendiente de implementar)");
}

/** @brief Refleja en el HUD el contador de regalos y carga especial. */
static void updateGiftCounter(void) {
    TRACE_FUNC();
    const u16 displayValue = giftCounter_getDisplayValue(&giftCounterBlink,
        giftsCollected, frameCounter);
    giftCounter_render(&giftCounterHUD, displayValue);
}

/** @brief Log de depuracion para el estado del scroll vertical. */
static void debugPrintScrollState(const char* context, s16 scrollStep) {
    s32 speedInt = F16_toInt(scrollSpeedPerFrame);
    s32 speedFrac = ((u32)(scrollSpeedPerFrame & 0xFFFF) * 1000) / 65536;
    s32 accInt = F16_toInt(scrollAccumulator);
    s32 accFrac = ((u32)(scrollAccumulator & 0xFFFF) * 1000) / 65536;

    kprintf("[DEBUG SCROLL] %s speed=%ld.%03ld acc=%ld.%03ld step=%d gifts=%u max=%u frame=%u",
        (context != NULL) ? context : "unknown",
        (long)speedInt, (long)speedFrac,
        (long)accInt, (long)accFrac,
        scrollStep,
        giftsCollected, maxGiftsCollected,
        frameCounter);
}

/**
 * @brief Devuelve el suelo de regalos permitido según el máximo alcanzado.
 */
static u16 giftsLossFloor(void) {
    TRACE_FUNC();
    if (maxGiftsCollected > GIFT_LOSS_MAX_OFFSET) {
        return maxGiftsCollected - GIFT_LOSS_MAX_OFFSET;
    }
    return 0;
}

/**
 * @brief Resta regalos aplicando el límite inferior parametrizable.
 * @param amount Cantidad a restar del contador actual.
 * @return TRUE si el contador llegó a modificarse.
 */
static u8 applyGiftLoss(u16 amount) {
    TRACE_FUNC();
    if (giftsCollected == 0) {
        return FALSE;
    }

    const u16 previousGifts = giftsCollected;
    const u16 minAllowed = giftsLossFloor();

    if (amount >= giftsCollected) {
        giftsCollected = 0;
    } else {
        giftsCollected -= amount;
    }

    if (giftsCollected < minAllowed) {
        giftsCollected = minAllowed;
    }

    if (giftsCollected < previousGifts) {
        giftCounter_startBlink(&giftCounterBlink, previousGifts, giftsCollected,
            GIFT_COUNTER_BLINK_INTERVAL_FRAMES);
        updateGiftCounter();
        return TRUE;
    }

    return FALSE;
}

/** @brief Procesa la recogida de un regalo y avanza la misión. */
static void collectGift(void) {
    TRACE_FUNC();
    XGM2_playPCM(snd_regalo_recogido, sizeof(snd_regalo_recogido), SOUND_PCM_CH_AUTO);
    giftsCollected++;
    if (giftsCollected > GIFT_COUNTER_MAX) giftsCollected = GIFT_COUNTER_MAX;
    if (giftsCollected > maxGiftsCollected) {
        maxGiftsCollected = giftsCollected;
    }
    giftsCharge++;

    if (giftsCollected == 3 && activeEnemyCount == 1) { // A los 3 regalos, spawnea el segundo enemigo
        activeEnemyCount = 2;
        spawnEnemy(&enemies[1]);
        kprintf("[DEBUG ENEMY] activeEnemyCount aumentó a 2");
    } else if (giftsCollected == 6 && activeEnemyCount == 2) { // A los 6 regalos, spawnea el tercer enemigo
        activeEnemyCount = 3;
        spawnEnemy(&enemies[2]);
        kprintf("[DEBUG ENEMY] activeEnemyCount aumentó a 3");
    }
    if (!secondTreeSpawned && (giftsCollected >= GIFTS_FOR_SECOND_TREE)) {
        secondTreeSpawned = TRUE;
        spawnTree(&trees[1]);
        if (trees[1].active) {
            kprintf("[DEBUG TREE] Segundo árbol activado (regalos=%u)", giftsCollected);
        }
    }

    kprintf("[DEBUG GIFT] collectGift giftsCollected=%u giftsCharge=%u", giftsCollected, giftsCharge);
    resetSpecialIfReady();
    updateGiftCounter();
    if (!phaseChangeRequested && (giftsCollected >= GIFT_COUNTER_MAX)) {
        phaseChangeRequested = TRUE;
        requestPhaseChange();
    }
}

/** @brief Resetea y oculta todos los enemigos activos. */
static void clearEnemies(void) {
    TRACE_FUNC();
    for (u8 i = 0; i < activeEnemyCount; i++) {
        spawnEnemy(&enemies[i]);
    }
}

/** @brief Alterna la visibilidad de Santa durante el parpadeo por colisión. */
static void setTreeCollisionVisibility(u8 visible) {
    if (santa.sprite) {
        SPR_setVisibility(santa.sprite, visible ? VISIBLE : HIDDEN);
    }
    if (collidedTree && collidedTree->sprite) {
        SPR_setVisibility(collidedTree->sprite, visible ? VISIBLE : HIDDEN);
    }
}

/** @brief Congela la animacion automatica del sprite de Santa. */
static void pauseSantaAnimation(void) {
    if (santaAnimationPaused) return;
    santaAnimationPaused = TRUE;
    if (santa.sprite) {
        SPR_setAutoAnimation(santa.sprite, FALSE);
    }
}

/** @brief Reanuda la animacion automatica del sprite de Santa. */
static void resumeSantaAnimation(void) {
    if (!santaAnimationPaused) return;
    santaAnimationPaused = FALSE;
    if (santa.sprite) {
        SPR_setAutoAnimation(santa.sprite, TRUE);
    }
}

/**
 * @brief Inicia el estado de colisión con un árbol.
 * @param tree Árbol con el que colisionó Santa.
 */
static void beginTreeCollision(SimpleActor *tree) {
    TRACE_FUNC();
    if (recoveringFromTree) return;

    collidedTree = tree;
    recoveringFromTree = TRUE;
    treeCollisionBlinkFrames = TREE_COLLISION_BLINK_FRAMES;
    treeCollisionVisible = TRUE;
    pauseSantaAnimation();

    clearEnemiesOnTreeCollision();
    clearElvesOnTreeCollision();
    clearOtherTreesOnCollision(tree);

    if (giftsCollected > 0) {
        applyGiftLoss(1);
    }

    XGM2_playPCM(snd_obstaculo_golpe, sizeof(snd_obstaculo_golpe), SOUND_PCM_CH_AUTO);
    setTreeCollisionVisibility(TRUE);
}

/** @brief Finaliza el parpadeo y estado de invulnerabilidad tras un choque. */
static void endTreeCollisionRecovery(void) {
    TRACE_FUNC();

    setTreeCollisionVisibility(TRUE);

    if (collidedTree) {
        spawnTree(collidedTree);
    }
    collidedTree = NULL;

    /* Reactivar cualquier árbol que quedara oculto tras la colisión. */
    for (u8 i = 0; i < NUM_TREES; i++) {
        if (!trees[i].active) {
            spawnTree(&trees[i]);
        }
    }

    santa.x = santaStartX;
    santa.y = santaStartY;
    santa.vx = 0;
    santa.vy = 0;
    if (santa.sprite) {
        SPR_setPosition(santa.sprite, santa.x, santa.y);
        SPR_setVisibility(santa.sprite, VISIBLE);
    }

    XGM2_playPCM(snd_santa_hohoho, sizeof(snd_santa_hohoho), SOUND_PCM_CH_AUTO);

    for (u8 i = 0; i < activeEnemyCount; i++) {
        spawnEnemy(&enemies[i]);
    }
    for (u8 i = 0; i < NUM_ELVES; i++) {
        if (elfRespawnTimer[i] == 0) {
            elfRespawnTimer[i] = randomFrameDelay(ELF_RESPAWN_DELAY_MIN_FRAMES, ELF_RESPAWN_DELAY_MAX_FRAMES);
        }
    }

    recoveringFromTree = TRUE;
}

/** @brief Gestiona el parpadeo y temporizador tras chocar con un árbol. */
static void updateTreeCollisionRecovery(void) {
    TRACE_FUNC();
    if (!recoveringFromTree) return;

    if (treeCollisionBlinkFrames > 0) {
        if ((treeCollisionBlinkFrames % TREE_COLLISION_BLINK_INTERVAL_FRAMES) == 0) {
            treeCollisionVisible = !treeCollisionVisible;
            setTreeCollisionVisibility(treeCollisionVisible);
        }
        treeCollisionBlinkFrames--;
        if (treeCollisionBlinkFrames == 0) {
            endTreeCollisionRecovery();
        }
        return;
    }

    resumeSantaAnimation();
    giftCounter_stopBlink(&giftCounterBlink);
    updateGiftCounter();
    recoveringFromTree = FALSE;
    return;
}

/**
 * @brief Calcula una posición de huida para el enemigo ladrón.
 * @param enemy Referencia al enemigo.
 * @param targetX Salida X objetivo.
 * @param targetY Salida Y objetivo.
 */
static void selectEnemyEscapeTarget(SimpleActor *enemy, s16 *targetX, s16 *targetY) {
    TRACE_FUNC();
    const s16 cornerX[4] = { -ENEMY_SIZE, SCREEN_WIDTH, -ENEMY_SIZE, SCREEN_WIDTH };
    const s16 cornerY[4] = { -ENEMY_SIZE, -ENEMY_SIZE, SCREEN_HEIGHT, SCREEN_HEIGHT };
    u32 bestDist = 0xFFFFFFFF;
    u8 bestIndex = 0;

    for (u8 i = 0; i < 4; i++) {
        s32 dx = (s32)cornerX[i] - (s32)enemy->x;
        s32 dy = (s32)cornerY[i] - (s32)enemy->y;
        u32 dist = (u32)(dx * dx + dy * dy);
        if (dist < bestDist) {
            bestDist = dist;
            bestIndex = i;
        }
    }

    *targetX = cornerX[bestIndex];
    *targetY = cornerY[bestIndex];
}

/** @brief Restaura estado tras finalizar la secuencia de robo. */
static void endEnemyStealSequence(void) {
    TRACE_FUNC();
    giftCounter_stopBlink(&giftCounterBlink);
    updateGiftCounter();
    enemyStealActive = FALSE;
    spawnEnemy(&enemies[enemyStealIndex]);
}

/**
 * @brief Activa la secuencia en la que un enemigo roba el regalo.
 * @param enemyIndex Índice del enemigo implicado.
 */
static void beginEnemyStealSequence(u8 enemyIndex) {
    if (enemyIndex >= NUM_ENEMIES) return;
    TRACE_FUNC();
    enemyStealActive = TRUE;
    enemyStealIndex = enemyIndex;
    selectEnemyEscapeTarget(&enemies[enemyIndex], &enemyEscapeTargetX, &enemyEscapeTargetY);
    if (enemies[enemyIndex].sprite) {
        SPR_setAnim(enemies[enemyIndex].sprite, 1);
        SPR_setAutoAnimation(enemies[enemyIndex].sprite, TRUE);
    }
    XGM2_playPCM(snd_elfo_robando, sizeof(snd_elfo_robando), SOUND_PCM_CH_AUTO);
}

/** @brief Mueve al enemigo ladrón y resuelve final de secuencia. */
static void updateEnemyStealSequence(void) {
    if (!enemyStealActive) return;
    TRACE_FUNC();
    SimpleActor *enemy = &enemies[enemyStealIndex];

    if (!enemy->active || (enemy->sprite == NULL)) {
        endEnemyStealSequence();
        return;
    }

    s16 dx = enemyEscapeTargetX - enemy->x;
    s16 dy = enemyEscapeTargetY - enemy->y;

    s16 stepX = 0;
    s16 stepY = 0;
    if (dx > 0) stepX = ENEMY_ESCAPE_SPEED;
    else if (dx < 0) stepX = -ENEMY_ESCAPE_SPEED;
    if (dy > 0) stepY = ENEMY_ESCAPE_SPEED;
    else if (dy < 0) stepY = -ENEMY_ESCAPE_SPEED;

    enemy->x += stepX;
    enemy->y += stepY;

    if (enemy->sprite) {
        SPR_setPosition(enemy->sprite, enemy->x, enemy->y);
    }

    if ((dx == 0) && (dy == 0)) {
        endEnemyStealSequence();
        return;
    }

    const u8 offScreen =
        (enemy->x < -ENEMY_SIZE) || (enemy->x > SCREEN_WIDTH) ||
        (enemy->y < -ENEMY_SIZE) || (enemy->y > SCREEN_HEIGHT);
    if (offScreen) {
        endEnemyStealSequence();
    }
}

typedef struct {
    Sprite* sprite;
    s16 bottom;
} DepthEntry;

/** @brief Ordena la profundidad de sprites según su base (eje Y). */
static void reorderDepthByBottom(void) {
    TRACE_FUNC();
    DepthEntry entries[NUM_TREES + NUM_ENEMIES + NUM_ELVES + NUM_ELVES + 1];
    u8 count = 0;

    if (santa.sprite) {
        entries[count].sprite = santa.sprite;
        entries[count].bottom = santa.y + SANTA_HEIGHT;
        count++;
    }

    for (u8 i = 0; i < NUM_TREES; i++) {
        if (trees[i].sprite && trees[i].active) {
            if (count >= sizeof(entries) / sizeof(entries[0])) break;
            entries[count].sprite = trees[i].sprite;
            entries[count].bottom = trees[i].y + TREE_SIZE;
            count++;
        }
    }

    for (u8 i = 0; i < NUM_ENEMIES; i++) {
        if (enemies[i].sprite && enemies[i].active) {
            if (count >= sizeof(entries) / sizeof(entries[0])) break;
            entries[count].sprite = enemies[i].sprite;
            entries[count].bottom = enemies[i].y + ENEMY_SIZE;
            count++;
        }
    }

    for (u8 i = 0; i < NUM_ELVES; i++) {
        if (elves[i].sprite && elves[i].active) {
            if (count >= sizeof(entries) / sizeof(entries[0])) break;
            entries[count].sprite = elves[i].sprite;
            entries[count].bottom = elves[i].y + ELF_SIZE;
            count++;
        }
    }

    for (u8 i = 0; i < NUM_ELVES; i++) {
        if (elfGiftSprites[i] && elfGiftActive[i]) {
            if (count >= sizeof(entries) / sizeof(entries[0])) break;
            entries[count].sprite = elfGiftSprites[i];
            entries[count].bottom = elfGiftPosY[i] + GIFT_SIZE;
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

    u16 depth = DEPTH_ACTORS_START;
    for (u8 i = 0; i < count; i++) {
        SPR_setDepth(entries[i].sprite, depth++);
    }
}

#if DEBUG_MODE
/** @brief Dibuja overlays de depuración en pantalla. */
static void renderDebug(void) {
    TRACE_FUNC();
}
#endif

/** @brief Inicializa recursos, estado y entidades de la fase de recogida. */
void minigamePickup_init(void) {
    TRACE_FUNC();
    audio_stop_music();
    gameCore_resetVideoState();
    giftsCollected = 0;
    maxGiftsCollected = 0;
    giftsCharge = 0;
    frameCounter = 0;
    scrollAccumulator = FIX16(0);
    giftCounterSpriteFirstRow = NULL;
    giftCounterSpriteSecondRow = NULL;
    phaseChangeRequested = FALSE;
    recoveringFromTree = FALSE;
    treeCollisionBlinkFrames = 0;
    treeCollisionVisible = TRUE;
    collidedTree = NULL;
    enemyStealActive = FALSE;
    enemyStealIndex = 0;
    enemyEscapeTargetX = 0;
    enemyEscapeTargetY = 0;
    musicStarted = FALSE;
    musicStartDelayFrames = MUSIC_START_DELAY_FRAMES;
    santaAnimationPaused = FALSE;
    scrollSpeedPerFrame = SCROLL_SPEED_MAX;
    secondTreeSpawned = FALSE;

    leftLimit = (SCREEN_WIDTH * FORBIDDEN_PERCENT) / 100;
    rightLimit = SCREEN_WIDTH - leftLimit;
    playableWidth = rightLimit - leftLimit - SANTA_WIDTH;
    santaMinY = 0;
    santaMaxY = SCREEN_HEIGHT - SANTA_HEIGHT; /* dejar sprite completo visible */
    santaInertia.accel = 1;
    santaInertia.friction = 1;
    santaInertia.frictionDelay = 3; /* frena cada 3 frames para aumentar la inercia */
    santaInertia.maxVelocity = 6;

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
    trackOffsetY = TRACK_LOOP_PX;
    if (mapTrack != NULL) {
        MAP_scrollTo(mapTrack, 0, trackOffsetY);
    }
    SYS_doVBlankProcess();

    snowEffect_init(&snowEffect, &globalTileIndex, 1, -4);

    santaStartX = leftLimit + (playableWidth / 4); /* centro aproximado de la mitad izquierda */
    santaStartY = santaMaxY;
    santa.x = santaStartX;
    santa.y = santaStartY;
    santa.vx = 0;
    santa.vy = 0;
    santa.specialReady = FALSE;
    santa.sprite = SPR_addSpriteSafe(&sprite_santa_car, santa.x, santa.y,
        TILE_ATTR(PAL_PLAYER, FALSE, FALSE, FALSE));
    SPR_setAnim(santa.sprite, 0);
    SPR_setDepth(santa.sprite, DEPTH_SANTA);
    SPR_setAutoAnimation(santa.sprite, TRUE);
    XGM2_playPCM(snd_santa_hohoho, sizeof(snd_santa_hohoho), SOUND_PCM_CH_AUTO);

    const s16 giftBaseX = SCREEN_WIDTH - HUD_MARGIN_PX - GIFT_COUNTER_SPRITE_WIDTH;
    const s16 giftBaseY = SCREEN_HEIGHT - GIFT_COUNTER_SPRITE_HEIGHT - HUD_MARGIN_PX;

    giftCounterSpriteFirstRow = SPR_addSpriteSafe(&sprite_icono_regalo,
        giftBaseX, giftBaseY - GIFT_COUNTER_ROW_OFFSET_Y,
        TILE_ATTR(PAL_EFFECT, FALSE, FALSE, FALSE));
    if (giftCounterSpriteFirstRow) {
        SPR_setAnim(giftCounterSpriteFirstRow, 0);
        SPR_setFrame(giftCounterSpriteFirstRow, 0);
        SPR_setDepth(giftCounterSpriteFirstRow, DEPTH_HUD + 1);
        SPR_setAutoAnimation(giftCounterSpriteFirstRow, FALSE);
    }

    giftCounterSpriteSecondRow = SPR_addSpriteSafe(&sprite_icono_regalo,
        giftBaseX + GIFT_COUNTER_SECOND_ROW_OFFSET_X, giftBaseY,
        TILE_ATTR(PAL_EFFECT, FALSE, FALSE, FALSE));
    if (giftCounterSpriteSecondRow) {
        SPR_setAnim(giftCounterSpriteSecondRow, 0);
        SPR_setFrame(giftCounterSpriteSecondRow, 0);
        SPR_setDepth(giftCounterSpriteSecondRow, DEPTH_HUD);
        SPR_setAutoAnimation(giftCounterSpriteSecondRow, FALSE);
    }

    giftCounter_initHUD(&giftCounterHUD, giftCounterSpriteFirstRow,
        giftCounterSpriteSecondRow, giftBaseX, giftBaseY,
        -GIFT_COUNTER_ROW_OFFSET_Y, GIFT_COUNTER_SECOND_ROW_OFFSET_X,
        DEPTH_HUD + 1, DEPTH_HUD, GIFT_COUNTER_ROW_SIZE, GIFT_COUNTER_MAX);
    giftCounter_stopBlink(&giftCounterBlink);

    updateGiftCounter();

    for (u8 i = 0; i < NUM_TREES; i++) {
        trees[i].sprite = NULL;
        trees[i].active = FALSE;
        trees[i].x = 0;
        trees[i].y = 0;
    }
    spawnTree(&trees[0]);
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
        elfGiftHasLanded[i] = FALSE;
        elfGiftPosX[i] = 0;
        elfGiftPosY[i] = 0;
        elfRespawnTimer[i] = randomFrameDelay(ELF_RESPAWN_DELAY_MIN_FRAMES, ELF_RESPAWN_DELAY_MAX_FRAMES);
        elfSide[i] = i % 2;
        elves[i].active = FALSE;
        kprintf("[ELF %d] Respawn inicial en %u frames", i, elfRespawnTimer[i]);
    }
    activeEnemyCount = 1;  // Empieza con 1 enemigo
    for (u8 i = 0; i < NUM_ENEMIES; i++) {
        enemies[i].sprite = NULL;
        if (i < activeEnemyCount) {
            spawnEnemy(&enemies[i]);  // Spawneamos solo los activos
        } else {
            enemies[i].active = FALSE;  // Los demás inactivos
        }
    }
}

/** @brief Actualiza entrada, físicas y lógica de la fase cada frame. */
void minigamePickup_update(void) {
    TRACE_FUNC();
    startMusicAfterHoHoHo();
    updateGiftCounter();
    updateTreeCollisionRecovery();
    if (recoveringFromTree) {
        frameCounter++;
        return;
    }
    if (enemyStealActive) {
        updateEnemyStealSequence();
        reorderDepthByBottom();
        frameCounter++;
        return;
    }

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
    SPR_setDepth(santa.sprite, DEPTH_SANTA);

    /* Caja de colisión reducida: solo los 40 px centrales (80 px de sprite) */
    const s16 santaHitX = santa.x + SANTA_HITBOX_PADDING;
    const s16 santaHitY = santa.y;
    const s16 santaHitW = SANTA_HITBOX_WIDTH;
    const s16 santaHitH = SANTA_HEIGHT;

    /* Scroll del fondo hacia abajo (caida) */
    scrollAccumulator += scrollSpeedPerFrame;
    s16 scrollStep = 0;
    while (scrollAccumulator >= FIX16(1)) {
        scrollStep++;
        scrollAccumulator -= FIX16(1);
    }
    if ((frameCounter & 31) == 0) {
        debugPrintScrollState("frame", scrollStep);
    }
    if (scrollStep > 0) {
        trackOffsetY -= scrollStep;
        if (trackOffsetY < 0) {
            trackOffsetY += TRACK_LOOP_PX;
        }
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
        if (gameCore_checkCollision(santaHitX, santaHitY, santaHitW, santaHitH,
                        trees[i].x + TREE_HITBOX_OFFSET_X, 
                        trees[i].y + TREE_HITBOX_OFFSET_Y, 
                        TREE_HITBOX_WIDTH, 
                        TREE_HITBOX_HEIGHT)) {
            beginTreeCollision(&trees[i]);
        }
        SPR_setPosition(trees[i].sprite, trees[i].x, trees[i].y);
    }

    for (u8 i = 0; i < NUM_ELVES; i++) {
        if (!elves[i].active) {
            hideElfMark(i);
            hideElfShadow(i);
            hideElfGift(i, FALSE);
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
        updateElfMark(i, santaHitX, santaHitY, santaHitW, santaHitH);
        SPR_setPosition(elves[i].sprite, elves[i].x, elves[i].y);
    }

    for (u8 i = 0; i < activeEnemyCount; i++) {
        if (!enemies[i].active) continue;
        enemies[i].y += scrollStep;
        if ((frameCounter % ENEMY_LATERAL_DELAY) == 0) {
            if (enemies[i].x < santa.x) enemies[i].x += ENEMY_LATERAL_SPEED;
            else if (enemies[i].x > santa.x) enemies[i].x -= ENEMY_LATERAL_SPEED;
        }
        if (enemies[i].y > SCREEN_HEIGHT) {
            spawnEnemy(&enemies[i]);
        }
        if (gameCore_checkCollision(
                santaHitX, santaHitY, santaHitW, santaHitH,
                enemies[i].x,
                enemies[i].y + (ENEMY_SIZE - ENEMY_HITBOX_HEIGHT),
                ENEMY_SIZE, ENEMY_HITBOX_HEIGHT)) {
            if (giftsCollected > 0) {
                applyGiftLoss(1);
                beginEnemyStealSequence(i);
                updateEnemyStealSequence();
                reorderDepthByBottom();
                frameCounter++;
                return;
            } else {
                XGM2_playPCM(snd_elfo_choque, sizeof(snd_elfo_choque), SOUND_PCM_CH_AUTO);
            }
            spawnEnemy(&enemies[i]);
        }
        SPR_setPosition(enemies[i].sprite, enemies[i].x, enemies[i].y);
    }

    reorderDepthByBottom();

    frameCounter++;
}

/** @brief Renderiza sprites, HUD y efectos de la fase de recogida. */
void minigamePickup_render(void) {
    TRACE_FUNC();
#if DEBUG_MODE
    renderDebug();
#endif

    SPR_update();
    SYS_doVBlankProcess();
}

/**
 * @brief Indica si se alcanzó el objetivo de regalos.
 * @return TRUE cuando se recolectan los regalos necesarios o hay cambio de fase.
 */
u8 minigamePickup_isComplete(void) {
    TRACE_FUNC();
    return (giftsCollected >= TARGET_GIFTS);
}

/** @brief Activa la música de fase tras reproducir el grito de Santa. */
static void startMusicAfterHoHoHo(void) {
    if (musicStarted) return;
    if (musicStartDelayFrames > 0) {
        musicStartDelayFrames--;
        return;
    }
    audio_play_phase1();
    musicStarted = TRUE;
}

/** @brief Limpia enemigos cuando Santa choca con un árbol. */
static void clearEnemiesOnTreeCollision(void) {
    enemyStealActive = FALSE;
    for (u8 i = 0; i < NUM_ENEMIES; i++) {
        enemies[i].active = FALSE;
        if (enemies[i].sprite) {
            SPR_setVisibility(enemies[i].sprite, HIDDEN);
        }
    }
}

/** @brief Oculta elfos y objetos cuando hay colisión con árbol. */
static void clearElvesOnTreeCollision(void) {
    for (u8 i = 0; i < NUM_ELVES; i++) {
        elves[i].active = FALSE;
        elfRespawnTimer[i] = randomFrameDelay(ELF_RESPAWN_DELAY_MIN_FRAMES, ELF_RESPAWN_DELAY_MAX_FRAMES);
        hideElfMark(i);
        hideElfShadow(i);
        hideElfGift(i, TRUE);
        if (elves[i].sprite) {
            SPR_setVisibility(elves[i].sprite, HIDDEN);
        }
    }
}

/** @brief Oculta cualquier árbol distinto al que ha colisionado. */
static void clearOtherTreesOnCollision(const SimpleActor *treeToKeep) {
    for (u8 i = 0; i < NUM_TREES; i++) {
        if (&trees[i] == treeToKeep) continue;
        trees[i].active = FALSE;
        if (trees[i].sprite) {
            SPR_setVisibility(trees[i].sprite, HIDDEN);
        }
    }
}
