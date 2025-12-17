/**
 * @file minigame_bells.c
 * @brief Fase 3: Campanadas (implementación completa y corregida).
 *
 * Recursos y paletas:
 * - `resources_bg.h`: mapa de fondo nevado y paleta asociada cargada en
 *   `mapBackground` para el plano B.
 * - `resources_sprites.h`: sprites de campanas, bombas, cañón y letras. Cada
 *   sprite incluye su propia paleta; las letras usan la misma paleta al pasar
 *   de color a blanco y negro para indicar progreso.
 * - `resources_sfx.h`: efectos de impacto y campana; no afectan a paletas.
 * - `audio_manager.h`/`resources_music.h`: música de la fase reutilizada en
 *   bucle mientras `gameTimer` no expira.
 */

#include <genesis.h>
#include "audio_manager.h"
#include "minigame_bells.h"
#include "resources_bg.h"
#include "resources_sfx.h"
#include "resources_sprites.h"
#include "snow_effect.h"

#define ENABLE_OPTIONAL_LETTER_SFX 0     /* Activa SFX opcional al colocar letras. */

#define NUM_BELLS 6                      /* Campanas móviles en caída. */
#define NUM_FIXED_BELLS 12               /* Campanas fijas del marcador. */
#define NUM_BOMBS 3                      /* Bombas concurrentes en pantalla. */
#define NUM_BULLETS 3                    /* Munición simultánea del cañón. */
#define NUM_LETTERS 8                    /* Letras que caen para formar el mensaje. */
#define NUM_TARGET_LETTERS 9             /* Letras totales del mensaje objetivo. */
#define FRAMES_BLINK 30                  /* Intervalo de parpadeo en frames. */
#define BULLET_VELOCITY 2                /* Velocidad vertical de cada disparo. */
#define CANNON_ACCEL 1                   /* Aceleración horizontal del cañón. */
#define CANNON_FRICTION 1                /* Fricción horizontal del cañón. */
#define CANNON_MAX_VEL 6                 /* Velocidad máxima del cañón. */
#define BULLET_COOLDOWN_FRAMES 30        /* Enfriamiento entre disparos. */
#define BUTTON_B_COOLDOWN_FRAMES 30      /* Enfriamiento entre acciones con B. */

#define LETTER_WIDTH 32                  /* Ancho de cada letra en píxeles. */
#define LETTER_HEIGHT 32                 /* Alto de cada letra en píxeles. */
#define LETTER_COLLISION_HEIGHT 20       /* Altura útil para colisiones de letra. */

enum {
    PHASE_BELLS = 0,
    PHASE_LETTERS = 1,
    PHASE_COMPLETED = 2,
};

/** @brief Campana móvil que cae desde la parte superior. */
typedef struct {
    Sprite* sprite;
    s16 x, y;
    s8 velocity;
    u8 blinkCounter;
    u8 isBlinking;
} Bell;

/** @brief Campana fija del marcador inferior. */
typedef struct {
    Sprite* sprite;
    u8 active;
} FixedBell;

/** @brief Bomba que penaliza al jugador si la impacta. */
typedef struct {
    Sprite* sprite;
    s16 x, y;
    s8 velocity;
    u8 blinkCounter;
    u8 isBlinking;
} Bomb;

/** @brief Proyectil disparado por el cañón del jugador. */
typedef struct {
    Sprite* sprite;
    u8 active;
    s16 x, y;
} Bullet;

/** @brief Letra descendente a la que hay que disparar. */
typedef struct {
    Sprite* sprite;
    s16 x, y;
    s8 velocity;
    u8 blinkCounter;
    u8 isBlinking;
} Letter;

static Bell bells[NUM_BELLS]; /**< Campanas móviles visibles en pantalla. */
static FixedBell fixedBells[NUM_FIXED_BELLS]; /**< Marcador inferior de progreso. */
static Bomb bombs[NUM_BOMBS]; /**< Bombas que penalizan al jugador. */
static Bullet bullets[NUM_BULLETS]; /**< Proyectiles activos en vuelo. */
static Letter letters[NUM_LETTERS]; /**< Letras descendentes objetivo. */
static Sprite* felizSprites[NUM_TARGET_LETTERS]; /**< Letras de la palabra FELIZ2025. */

static Sprite* playerCannon; /**< Sprite del cañón controlable. */
static s16 cannonX; /**< Posición horizontal del cañón. */
static s8 cannonVelocity; /**< Velocidad actual aplicada al cañón. */
static u8 cannonFiring; /**< Estado de disparo en curso. */
static u8 currentPhase; /**< Subfase interna (campanas/letras/fin). */
static u8 currentLetterIndex; /**< Índice de letra que debe recogerse ahora. */
static u8 victoryTriggered; /**< Evita repetir la secuencia de victoria. */
static s8 buttonBCooldown; /**< Retraso entre disparos con botón B. */

static u16 bellsCompleted; /**< Campanas encendidas correctamente. */
static u16 activeBullets; /**< Proyectiles actualmente en pantalla. */
static u16 frameCounter; /**< Contador general para timings. */
static s8 bulletCooldown; /**< Ventana de recarga del cañón. */

static GameTimer gameTimer; /**< Temporizador para derrota por tiempo. */
static Map *mapBackground; /**< Mapa de fondo asignado al plano B. */
static SnowEffect snowEffect; /**< Partículas de nieve reutilizadas. */
static const GameInertia cannonInertia = { CANNON_ACCEL, CANNON_FRICTION, 1, CANNON_MAX_VEL }; /**< Configuración de inercia del cañón. */
static const SpriteDefinition* const letterSpritesColor[NUM_LETTERS] = {
    &sprite_letra_f, &sprite_letra_e, &sprite_letra_l, &sprite_letra_i,
    &sprite_letra_z, &sprite_letra_2, &sprite_letra_0, &sprite_letra_6
}; /**< Versiones a color de las letras descendentes. */
static const SpriteDefinition* const letterSpritesBW[NUM_LETTERS] = {
    &sprite_letra_bn_f, &sprite_letra_bn_e, &sprite_letra_bn_l, &sprite_letra_bn_i,
    &sprite_letra_bn_z, &sprite_letra_bn_2, &sprite_letra_bn_0, &sprite_letra_bn_6
}; /**< Versiones en blanco y negro para indicar letras ya conseguidas. */
static const SpriteDefinition* const felizSpritesColor[NUM_TARGET_LETTERS] = {
    &sprite_letra_f, &sprite_letra_e, &sprite_letra_l, &sprite_letra_i,
    &sprite_letra_z, &sprite_letra_2, &sprite_letra_0, &sprite_letra_2, &sprite_letra_6
}; /**< Muestra de referencia de la palabra final coloreada. */
static const SpriteDefinition* const felizSpritesBW[NUM_TARGET_LETTERS] = {
    &sprite_letra_bn_f, &sprite_letra_bn_e, &sprite_letra_bn_l, &sprite_letra_bn_i,
    &sprite_letra_bn_z, &sprite_letra_bn_2, &sprite_letra_bn_0, &sprite_letra_bn_2, &sprite_letra_bn_6
}; /**< Variante monocroma del mensaje final. */

static void detectarColisionesBala(Bullet* bala);
static void initLetter(Letter* letter, u8 index);
static void resetLetter(Letter* letter);
static void updateLetter(Letter* letter);
static void initFeliz2025(void);
static void updateFeliz2025Blink(void);
static void handleLetterCollision(Bullet* bullet, Letter* letter);
static void handleBombCollision(Bullet* bullet, Bomb* bomb);
static void handleBellCollision(Bullet* bullet, Bell* bell);
static void startLettersPhase(void);

/**
 * @brief Inicializa una campana móvil con posición aleatoria.
 * @param bell Campana a configurar.
 * @param index Índice para desfasar posición inicial.
 */
static void initBell(Bell* bell, u8 index) {
    s16 posX = random() % (SCREEN_WIDTH - 32);

    bell->sprite = SPR_addSpriteSafe(&sprite_campana, posX, -32,
        TILE_ATTR(PAL_ENEMY, FALSE, FALSE, FALSE));

    bell->x = posX;
    bell->y = -32 - (random() % 100);
    bell->velocity = (random() % 2) + 1;
    bell->blinkCounter = 0;
    bell->isBlinking = FALSE;
    (void)index;
}

/** @brief Reinicia posición y estado de una campana móvil. */
static void resetBell(Bell* bell) {
    bell->x = random() % (SCREEN_WIDTH - 32);
    bell->y = -32 - (random() % 100);
    bell->isBlinking = FALSE;
    bell->blinkCounter = 0;
    SPR_setVisibility(bell->sprite, VISIBLE);
    SPR_setPosition(bell->sprite, bell->x, bell->y);
}

/**
 * @brief Actualiza la animación y reciclado de una campana móvil.
 * @param bell Campana a actualizar.
 */
static void updateBell(Bell* bell) {
    if (bell->y + 32 >= SCREEN_HEIGHT - 64 && !bell->isBlinking) {
        bell->isBlinking = TRUE;
        bell->blinkCounter = FRAMES_BLINK;
    }

    if (bell->isBlinking) {
        if (bell->blinkCounter % 2 == 0) {
            SPR_setVisibility(bell->sprite, VISIBLE);
        } else {
            SPR_setVisibility(bell->sprite, HIDDEN);
        }

        bell->blinkCounter--;
        if (bell->blinkCounter <= 0) {
            bell->isBlinking = FALSE;
            SPR_setVisibility(bell->sprite, VISIBLE);
            resetBell(bell);
        }
        return;
    }

    if ((frameCounter % bell->velocity) == 0) {
        bell->y++;
    }

    if (bell->y > SCREEN_HEIGHT) {
        resetBell(bell);
    }

    SPR_setPosition(bell->sprite, bell->x, bell->y);
}

/** @brief Crea el conjunto de campanas fijas del marcador inferior. */
static void initFixedBells(void) {
    s16 separador_x = 24, separador_y = 16;
    s16 x = 0;
    s16 y = SCREEN_HEIGHT - 3 * separador_y - 16;

    for (u8 i = 0; i < NUM_FIXED_BELLS; i++) {
        fixedBells[i].sprite = SPR_addSprite(&sprite_campana_bn, x, y,
            TILE_ATTR(PAL_ENEMY, FALSE, FALSE, FALSE));
        fixedBells[i].active = FALSE;

        x += separador_x;
        if (i == 3 || i == 7) {
            y += separador_y;
            x = 0;
        }
    }
}

/** @brief Actualiza la animación de la siguiente campanilla fija a iluminar. */
static void updateFixedBells(void) {
    if (bellsCompleted < NUM_FIXED_BELLS) {
        u8 usarColor = (frameCounter % 4) < 2;
        SPR_setDefinition(fixedBells[bellsCompleted].sprite,
            usarColor ? &sprite_campana : &sprite_campana_bn);
    }
}

/**
 * @brief Inicializa una bomba móvil.
 * @param bomb Bomba a configurar.
 * @param index Índice para variar la posición inicial.
 */
static void initBomb(Bomb* bomb, u8 index) {
    s16 posX = random() % (SCREEN_WIDTH - 32);

    bomb->sprite = SPR_addSpriteSafe(&sprite_bomba, posX, -32,
        TILE_ATTR(PAL_EFFECT, FALSE, FALSE, FALSE));

    bomb->x = posX;
    bomb->y = -32;
    bomb->velocity = (random() % 3) + 1;
    bomb->blinkCounter = 0;
    bomb->isBlinking = FALSE;
    bomb->y -= (index * 8);
}

/** @brief Reinicia posición y estado de una bomba. */
static void resetBomb(Bomb* bomb) {
    bomb->x = random() % (SCREEN_WIDTH - 32);
    bomb->y = -32;
    bomb->isBlinking = FALSE;
    bomb->blinkCounter = 0;
    SPR_setVisibility(bomb->sprite, VISIBLE);
    SPR_setPosition(bomb->sprite, bomb->x, bomb->y);
}

/**
 * @brief Actualiza movimiento y blinking de una bomba.
 * @param bomb Bomba a actualizar.
 */
static void updateBomb(Bomb* bomb) {
    if (bomb->y + 32 >= SCREEN_HEIGHT - 64 && !bomb->isBlinking) {
        bomb->isBlinking = TRUE;
        bomb->blinkCounter = FRAMES_BLINK;
    }

    if (bomb->isBlinking) {
        if (bomb->blinkCounter % 2 == 0) {
            SPR_setVisibility(bomb->sprite, VISIBLE);
        } else {
            SPR_setVisibility(bomb->sprite, HIDDEN);
        }

        bomb->blinkCounter--;
        if (bomb->blinkCounter <= 0) {
            bomb->isBlinking = FALSE;
            SPR_setVisibility(bomb->sprite, VISIBLE);
            resetBomb(bomb);
        }
        return;
    }

    if ((frameCounter % bomb->velocity) == 0) {
        bomb->y++;
    }

    if (bomb->y > SCREEN_HEIGHT) {
        resetBomb(bomb);
    }

    SPR_setPosition(bomb->sprite, bomb->x, bomb->y);
}

/**
 * @brief Inicializa una letra descendente.
 * @param letter Letra a configurar.
 * @param index Índice para seleccionar sprite y variar posición.
 */
static void initLetter(Letter* letter, u8 index) {
    s16 posX = random() % (SCREEN_WIDTH - LETTER_WIDTH);

    letter->sprite = SPR_addSpriteSafe(letterSpritesColor[index], posX, -32,
        TILE_ATTR(PAL_ENEMY, FALSE, FALSE, FALSE));

    letter->x = posX;
    letter->y = -32 - (random() % 100);
    letter->velocity = (random() % 4) + 1;
    letter->blinkCounter = 0;
    letter->isBlinking = FALSE;
}

/** @brief Reinicia posición y estado de una letra. */
static void resetLetter(Letter* letter) {
    letter->x = random() % (SCREEN_WIDTH - LETTER_WIDTH);
    letter->y = -32 - (random() % 100);
    letter->isBlinking = FALSE;
    letter->blinkCounter = 0;
    SPR_setVisibility(letter->sprite, VISIBLE);
    SPR_setPosition(letter->sprite, letter->x, letter->y);
}

/**
 * @brief Actualiza movimiento y parpadeo de una letra.
 * @param letter Letra a actualizar.
 */
static void updateLetter(Letter* letter) {
    if (letter->y + LETTER_HEIGHT >= SCREEN_HEIGHT - 64 && !letter->isBlinking) {
        letter->isBlinking = TRUE;
        letter->blinkCounter = FRAMES_BLINK;
    }

    if (letter->isBlinking) {
        SPR_setVisibility(letter->sprite, (letter->blinkCounter % 2) ? HIDDEN : VISIBLE);
        letter->blinkCounter--;
        if (letter->blinkCounter <= 0) {
            letter->isBlinking = FALSE;
            SPR_setVisibility(letter->sprite, VISIBLE);
            resetLetter(letter);
        }
        return;
    }

    if ((frameCounter % letter->velocity) == 0) {
        letter->y++;
    }

    if (letter->y > SCREEN_HEIGHT) {
        resetLetter(letter);
    }

    SPR_setPosition(letter->sprite, letter->x, letter->y);
}

/** @brief Inicializa el pool de balas disparables. */
static void initBullets(void) {
    for (u8 i = 0; i < NUM_BULLETS; i++) {
        bullets[i].sprite = NULL;
        bullets[i].active = FALSE;
        bullets[i].x = 0;
        bullets[i].y = 0;
    }
    activeBullets = 0;
}

/** @brief Dispara una nueva bala si hay hueco libre en el pool. */
static void fireBullet(void) {
    if (activeBullets >= NUM_BULLETS) return;

    XGM2_playPCM(snd_canon, sizeof(snd_canon), SOUND_PCM_CH_AUTO);

    for (u8 i = 0; i < NUM_BULLETS; i++) {
        if (!bullets[i].active) {
            s16 x = cannonX + (80 / 2) - 4;
            s16 y = SCREEN_HEIGHT - 64 + 20;

            bullets[i].sprite = SPR_addSpriteSafe(&sprite_bola_confeti,
                x, y, TILE_ATTR(PAL_EFFECT, TRUE, FALSE, FALSE));

            bullets[i].active = TRUE;
            bullets[i].x = x;
            bullets[i].y = y;
            activeBullets++;
            break;
        }
    }
}

/** @brief Actualiza todas las balas activas, reciclándolas cuando salen de pantalla. */
static void updateBullets(void) {
    for (u8 i = 0; i < NUM_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].y -= BULLET_VELOCITY;

            if (bullets[i].y < -8) {
                SPR_releaseSprite(bullets[i].sprite);
                bullets[i].sprite = NULL;
                bullets[i].active = FALSE;
                activeBullets--;
                continue;
            }

            detectarColisionesBala(&bullets[i]);

            if (bullets[i].active) {
                SPR_setPosition(bullets[i].sprite, bullets[i].x, bullets[i].y);
            }
        }
    }
}

/** @brief Libera una bala activa y la marca como disponible. */
static void desactivarBala(Bullet* bala) {
    if (bala->active) {
        SPR_releaseSprite(bala->sprite);
        bala->sprite = NULL;
        bala->active = FALSE;
        activeBullets--;
    }
}

/**
 * @brief Detecta impactos de una bala sobre campanas o bombas.
 * @param bala Bala a comprobar.
 */
static void detectarColisionesBala(Bullet* bala) {
    s16 balaCentroX = bala->x + 4;
    s16 balaCentroY = bala->y + 4;

    if (currentPhase == PHASE_COMPLETED) return;

    if (currentPhase == PHASE_BELLS) {
        for (u8 i = 0; i < NUM_BELLS; i++) {
            if (bells[i].isBlinking) continue;

            if (balaCentroX >= bells[i].x &&
                balaCentroX < bells[i].x + LETTER_WIDTH &&
                balaCentroY >= bells[i].y + 6 &&
                balaCentroY < bells[i].y + 26) {
                handleBellCollision(bala, &bells[i]);
                return;
            }
        }
    } else if (currentPhase == PHASE_LETTERS) {
        for (u8 i = 0; i < NUM_LETTERS; i++) {
            if (letters[i].isBlinking) continue;

            if (balaCentroX >= letters[i].x &&
                balaCentroX < letters[i].x + LETTER_WIDTH &&
                balaCentroY >= letters[i].y + (LETTER_HEIGHT - LETTER_COLLISION_HEIGHT) / 2 &&
                balaCentroY < letters[i].y + (LETTER_HEIGHT + LETTER_COLLISION_HEIGHT) / 2) {
                handleLetterCollision(bala, &letters[i]);
                return;
            }
        }
    }

    for (u8 i = 0; i < NUM_BOMBS; i++) {
        if (bombs[i].isBlinking) continue;

        if (balaCentroX >= bombs[i].x &&
            balaCentroX < bombs[i].x + LETTER_WIDTH &&
            balaCentroY >= bombs[i].y + 6 &&
            balaCentroY < bombs[i].y + 26) {
            handleBombCollision(bala, &bombs[i]);
            return;
        }
    }
}

/**
 * @brief Maneja el impacto de bala sobre una campana.
 * @param bullet Bala implicada (puede ser NULL para simulaciones).
 * @param bell Campana alcanzada.
 */
static void handleBellCollision(Bullet* bullet, Bell* bell) {
    if (bullet) desactivarBala(bullet);

    XGM2_playPCM(snd_campana, sizeof(snd_campana), SOUND_PCM_CH2);

    bell->isBlinking = TRUE;
    bell->blinkCounter = FRAMES_BLINK;

    if (bellsCompleted < NUM_FIXED_BELLS) {
        SPR_setDefinition(fixedBells[bellsCompleted].sprite, &sprite_campana);
        fixedBells[bellsCompleted].active = TRUE;
        bellsCompleted++;
    }
}

/**
 * @brief Calcula el índice de letra objetivo según el progreso actual.
 * @return Índice de la letra que se debe acertar en esta ronda.
 */
static u8 getTargetLetterIndex(void) {
    if (currentLetterIndex < 7) {
        return currentLetterIndex;
    } else if (currentLetterIndex == 7) {
        return 5; /* Segundo "2" */
    }
    return 7; /* "5" final */
}

/**
 * @brief Procesa la colisión de bala con una letra.
 * @param bullet Bala implicada (puede ser NULL para simulaciones).
 * @param letter Letra alcanzada.
 */
static void handleLetterCollision(Bullet* bullet, Letter* letter) {
    if (bullet) desactivarBala(bullet);

    u8 targetIndex = getTargetLetterIndex();
    u8 hitIndex = 0xFF;
    for (u8 i = 0; i < NUM_LETTERS; i++) {
        if (&letters[i] == letter) {
            hitIndex = i;
            break;
        }
    }

    if (hitIndex == 0xFF) return;

    if (hitIndex == targetIndex) {
        #if ENABLE_OPTIONAL_LETTER_SFX
        XGM2_playPCM(snd_letra_ok, sizeof(snd_letra_ok), SOUND_PCM_CH_AUTO);
        #endif
        SPR_setDefinition(felizSprites[currentLetterIndex], felizSpritesColor[targetIndex]);
        currentLetterIndex++;

        if (currentLetterIndex >= NUM_TARGET_LETTERS) {
            victoryTriggered = TRUE;
            currentPhase = PHASE_COMPLETED;
        #if ENABLE_OPTIONAL_LETTER_SFX
        XGM2_playPCM(snd_victoria, sizeof(snd_victoria), SOUND_PCM_CH_AUTO);
        #endif
        }
    } else {
        #if ENABLE_OPTIONAL_LETTER_SFX
        XGM2_playPCM(snd_letra_no, sizeof(snd_letra_no), SOUND_PCM_CH_AUTO);
        #endif
    }

    letter->isBlinking = TRUE;
    letter->blinkCounter = FRAMES_BLINK;
    SPR_setDefinition(letter->sprite, letterSpritesBW[hitIndex]);
}

/**
 * @brief Gestiona el impacto de una bomba durante la fase activa.
 * @param bullet Bala implicada (puede ser NULL para simulaciones).
 * @param bomb Bomba impactada.
 */
static void handleBombCollision(Bullet* bullet, Bomb* bomb) {
    (void)bomb;
    XGM2_playPCM(snd_bomba, sizeof(snd_bomba), SOUND_PCM_CH_AUTO);

    if (bullet) desactivarBala(bullet);

    for (u8 j = 0; j < NUM_BOMBS; j++) {
        bombs[j].isBlinking = TRUE;
        bombs[j].blinkCounter = FRAMES_BLINK;
    }

    if (currentPhase == PHASE_BELLS) {
        for (u8 j = 0; j < NUM_BELLS; j++) {
            bells[j].isBlinking = TRUE;
            bells[j].blinkCounter = FRAMES_BLINK;
        }

        bellsCompleted = 0;
        for (u8 j = 0; j < NUM_FIXED_BELLS; j++) {
            SPR_setDefinition(fixedBells[j].sprite, &sprite_campana_bn);
        }
    } else if (currentPhase == PHASE_LETTERS) {
        for (u8 j = 0; j < NUM_LETTERS; j++) {
            letters[j].isBlinking = TRUE;
            letters[j].blinkCounter = FRAMES_BLINK;
        }

        currentLetterIndex = 0;
        for (u8 j = 0; j < NUM_TARGET_LETTERS; j++) {
            SPR_setDefinition(felizSprites[j], felizSpritesBW[j]);
        }
    }
}

/** @brief Crea los indicadores de "FELIZ2025" en la parte inferior. */
static void initFeliz2025(void) {
    s16 x = 0;
    s16 y = SCREEN_HEIGHT - 4 - LETTER_HEIGHT;

    felizSprites[0] = SPR_addSpriteSafe(&sprite_letra_bn_f, x, y, TILE_ATTR(PAL_ENEMY, FALSE, FALSE, FALSE));
    x += LETTER_WIDTH / 2;
    felizSprites[1] = SPR_addSpriteSafe(&sprite_letra_bn_e, x + 4, y, TILE_ATTR(PAL_ENEMY, FALSE, FALSE, FALSE));
    x += LETTER_WIDTH / 2;
    felizSprites[2] = SPR_addSpriteSafe(&sprite_letra_bn_l, x + 4, y, TILE_ATTR(PAL_ENEMY, FALSE, FALSE, FALSE));
    x += LETTER_WIDTH / 2;
    felizSprites[3] = SPR_addSpriteSafe(&sprite_letra_bn_i, x, y, TILE_ATTR(PAL_ENEMY, FALSE, FALSE, FALSE));
    x += LETTER_WIDTH / 2;
    felizSprites[4] = SPR_addSpriteSafe(&sprite_letra_bn_z, x, y, TILE_ATTR(PAL_ENEMY, FALSE, FALSE, FALSE));
    x += 20;
    felizSprites[5] = SPR_addSpriteSafe(&sprite_letra_bn_2, x, y, TILE_ATTR(PAL_ENEMY, FALSE, FALSE, FALSE));
    x += LETTER_WIDTH / 2;
    felizSprites[6] = SPR_addSpriteSafe(&sprite_letra_bn_0, x, y, TILE_ATTR(PAL_ENEMY, FALSE, FALSE, FALSE));
    x += LETTER_WIDTH / 2;
    felizSprites[7] = SPR_addSpriteSafe(&sprite_letra_bn_2, x, y, TILE_ATTR(PAL_ENEMY, FALSE, FALSE, FALSE));
    x += LETTER_WIDTH / 2;
    felizSprites[8] = SPR_addSpriteSafe(&sprite_letra_bn_6, x, y, TILE_ATTR(PAL_ENEMY, FALSE, FALSE, FALSE));
}

/** @brief Hace parpadear la letra objetivo de "FELIZ2025". */
static void updateFeliz2025Blink(void) {
    if (currentPhase != PHASE_LETTERS || currentLetterIndex >= NUM_TARGET_LETTERS) return;

    u8 useColor = (frameCounter % 4) < 2;
    SPR_setDefinition(felizSprites[currentLetterIndex], useColor ? felizSpritesColor[currentLetterIndex] : felizSpritesBW[currentLetterIndex]);
}

/** @brief Transición de campanas a letras replicando la versión 2025. */
static void startLettersPhase(void) {
    currentPhase = PHASE_LETTERS;
            #if ENABLE_OPTIONAL_LETTER_SFX
            XGM2_playPCM(snd_aplausos, sizeof(snd_aplausos), SOUND_PCM_CH3);
            #endif

    for (u8 i = 0; i < NUM_BELLS; i++) {
        if (bells[i].sprite) {
            SPR_releaseSprite(bells[i].sprite);
            bells[i].sprite = NULL;
        }
    }
    for (u8 i = 0; i < NUM_FIXED_BELLS; i++) {
        if (fixedBells[i].sprite) {
            SPR_releaseSprite(fixedBells[i].sprite);
            fixedBells[i].sprite = NULL;
        }
    }

    for (u8 i = 0; i < NUM_LETTERS; i++) {
        initLetter(&letters[i], i);
    }
    initFeliz2025();
    currentLetterIndex = 0;
}

/** @brief Configura recursos, sprites y estado inicial de la fase. */
void minigameBells_init(void) {
    VDP_setScreenWidth320();
    VDP_setScreenHeight224();

    gameCore_resetTileIndex();

    SPR_init();
    JOY_init();

    if (sprite_campana.palette) {
        PAL_setPalette(PAL_ENEMY, sprite_campana.palette->data, CPU);
    }
    if (sprite_bomba.palette) {
        PAL_setPalette(PAL_EFFECT, sprite_bomba.palette->data, CPU);
    }
    if (image_fondo_pal.data) {
        PAL_setPalette(PAL_COMMON, image_fondo_pal.data, CPU);
    }
    if (sprite_canon.palette) {
        PAL_setPalette(PAL_PLAYER, sprite_canon.palette->data, CPU);
    }

    VDP_setBackgroundColor(0);

    /* Cargar fondo */
    VDP_loadTileSet(&image_fondo_tile, globalTileIndex, CPU);
    mapBackground = MAP_create(&image_fondo_map, BG_B,
        TILE_ATTR_FULL(PAL_COMMON, FALSE, FALSE, FALSE, globalTileIndex));
    globalTileIndex += image_fondo_tile.numTile;
    MAP_scrollTo(mapBackground, 0, 0);

    snowEffect_init(&snowEffect, &globalTileIndex, 2, -1);

    /* Música */
    audio_play_phase3();

    /* Campanas */
    for (u8 i = 0; i < NUM_BELLS; i++) {
        initBell(&bells[i], i);
    }

    /* Campanillas fijas */
    initFixedBells();

    /* Bombas */
    for (u8 i = 0; i < NUM_BOMBS; i++) {
        initBomb(&bombs[i], i);
    }

    /* Cañón */
    cannonX = (SCREEN_WIDTH - 80) / 2 + 80;
    playerCannon = SPR_addSpriteSafe(&sprite_canon, cannonX,
        SCREEN_HEIGHT - 64,
        TILE_ATTR(PAL_PLAYER, TRUE, FALSE, FALSE));
    SPR_setDepth(playerCannon, SPR_MIN_DEPTH);
    SPR_setAnim(playerCannon, 0);
    SPR_setAnimationLoop(playerCannon, FALSE);

    /* Balas */
    initBullets();

    /* Timer */
    gameCore_initTimer(&gameTimer, 0);

    /* Variables */
    bellsCompleted = 0;
    frameCounter = 0;
    cannonVelocity = 0;
    cannonFiring = FALSE;
    bulletCooldown = 0;
    buttonBCooldown = 0;
    currentPhase = PHASE_BELLS;
    currentLetterIndex = 0;
    victoryTriggered = FALSE;
}

/** @brief Actualiza entrada, física y entidades de la fase. */
void minigameBells_update(void) {
    u16 input = gameCore_readInput();
    s8 inputDirX = 0;
    if (input & BUTTON_LEFT) inputDirX = -1;
    else if (input & BUTTON_RIGHT) inputDirX = 1;

    if (currentPhase == PHASE_BELLS && bellsCompleted >= NUM_FIXED_BELLS) {
        startLettersPhase();
    }

    /* Cañón */
    gameCore_applyInertiaAxis(&cannonX, &cannonVelocity, -32, SCREEN_WIDTH - 32, inputDirX, frameCounter, &cannonInertia);
    SPR_setPosition(playerCannon, cannonX, SCREEN_HEIGHT - 64);

    /* Disparos */
    if ((input & BUTTON_A) && bulletCooldown <= 0 && currentPhase != PHASE_COMPLETED) {
        cannonFiring = TRUE;
        SPR_setAnim(playerCannon, 1);
        fireBullet();
        bulletCooldown = BULLET_COOLDOWN_FRAMES;
    }

    /* Ataque alternativo con botón B (impacto directo). */
    if ((input & BUTTON_B) && buttonBCooldown <= 0 && currentPhase != PHASE_COMPLETED) {
        buttonBCooldown = BUTTON_B_COOLDOWN_FRAMES;
        if (currentPhase == PHASE_BELLS) {
            s16 lowestY = -32;
            Bell* targetBell = NULL;
            for (u8 i = 0; i < NUM_BELLS; i++) {
                if (!bells[i].isBlinking && bells[i].y > lowestY) {
                    lowestY = bells[i].y;
                    targetBell = &bells[i];
                }
            }
            if (targetBell) {
                handleBellCollision(NULL, targetBell);
            }
        } else if (currentPhase == PHASE_LETTERS) {
            u8 targetIndex = getTargetLetterIndex();
            for (u8 i = 0; i < NUM_LETTERS; i++) {
                if (i == targetIndex && !letters[i].isBlinking) {
                    handleLetterCollision(NULL, &letters[i]);
                    break;
                }
            }
        }
    }

    if (cannonFiring) {
        if (SPR_isAnimationDone(playerCannon)) {
            cannonFiring = FALSE;
            SPR_setAnim(playerCannon, 0);
        }
    }

    /* Paralaje */
    snowEffect_update(&snowEffect, frameCounter);

    /* Actualizar objetos */
    if (currentPhase == PHASE_BELLS) {
        for (u8 i = 0; i < NUM_BELLS; i++) {
            updateBell(&bells[i]);
        }
        updateFixedBells();
    } else if (currentPhase == PHASE_LETTERS) {
        for (u8 i = 0; i < NUM_LETTERS; i++) {
            updateLetter(&letters[i]);
        }
        updateFeliz2025Blink();
    }

    for (u8 i = 0; i < NUM_BOMBS; i++) {
        updateBomb(&bombs[i]);
    }

    updateBullets();

    /* Timer y contadores */
    gameCore_updateTimer(&gameTimer);
    frameCounter++;
    if (bulletCooldown > 0) bulletCooldown--;
    if (buttonBCooldown > 0) buttonBCooldown--;
}

/** @brief Renderiza sprites y sincroniza con VBlank. */
void minigameBells_render(void) {
    SPR_update();
    SYS_doVBlankProcess();
}

/**
 * @brief Comprueba si se completó el objetivo de campanas iluminadas.
 * @return TRUE cuando se activaron todas las campanillas fijas.
 */
u8 minigameBells_isComplete(void) {
    return (currentPhase == PHASE_COMPLETED && victoryTriggered);
}
