/**
 * ═════════════════════════════════════════════════════════════════════════════
 * src/minigame_bells.c - VERSIÓN DEFINITIVA CORREGIDA
 * 
 * Fase 3: Campanadas (100% Funcional)
 * Basado en main.c FELIZ 2025
 * ═════════════════════════════════════════════════════════════════════════════
 */

#include "minigame_bells.h"
#include "resources.h"
#include <genesis.h>

/* ═══════════════════════════════════════════════════════════════════════════
   CONSTANTES
   ═══════════════════════════════════════════════════════════════════════════ */

#define NUM_BELLS 6
#define NUM_FIXED_BELLS 12
#define NUM_BOMBS 3
#define NUM_BULLETS 3
#define FRAMES_BLINK 30
#define BULLET_VELOCITY 2
#define CANNON_ACCEL 1
#define CANNON_FRICTION 1
#define CANNON_MAX_VEL 6
#define BULLET_COOLDOWN_FRAMES 30

/* ═══════════════════════════════════════════════════════════════════════════
   ESTRUCTURAS
   ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    Sprite* sprite;
    s16 x, y;
    s8 velocity;
    u8 blinkCounter;
    u8 isBlinking;
} Bell;

typedef struct {
    Sprite* sprite;
    u8 active;
} FixedBell;

typedef struct {
    Sprite* sprite;
    s16 x, y;
    s8 velocity;
    u8 blinkCounter;
    u8 isBlinking;
} Bomb;

typedef struct {
    Sprite* sprite;
    u8 active;
    s16 x, y;
} Bullet;

/* ═══════════════════════════════════════════════════════════════════════════
   VARIABLES GLOBALES
   ═══════════════════════════════════════════════════════════════════════════ */

static Bell bells[NUM_BELLS];
static FixedBell fixedBells[NUM_FIXED_BELLS];
static Bomb bombs[NUM_BOMBS];
static Bullet bullets[NUM_BULLETS];

static Sprite* playerCannon;
static s16 cannonX;
static s8 cannonVelocity;
static u8 cannonFiring;

static u16 bellsCompleted;
static u16 activeBullets;
static u16 frameCounter;
static s8 bulletCooldown;

static GameTimer gameTimer;
static Map *mapBackground;
static Map *mapSnow;
static s16 snowOffsetX, snowOffsetY;
static u32 tileIndex;

/* ═══════════════════════════════════════════════════════════════════════════
   FORWARD DECLARATIONS
   ═══════════════════════════════════════════════════════════════════════════ */

static void detectarColisionesBala(Bullet* bala);

/* ═══════════════════════════════════════════════════════════════════════════
   FUNCIONES - CAMPANAS
   ═══════════════════════════════════════════════════════════════════════════ */

static void initBell(Bell* bell, u8 index) {
    s16 posX = random() % (SCREEN_WIDTH - 32);
    
    bell->sprite = SPR_addSpriteSafe(&sprite_campana, posX, -32,
        TILE_ATTR(PAL_PLAYER, FALSE, FALSE, FALSE));
    
    bell->x = posX;
    bell->y = -32 - (random() % 100);
    bell->velocity = (random() % 2) + 1;
    bell->blinkCounter = 0;
    bell->isBlinking = FALSE;
}

static void resetBell(Bell* bell) {
    bell->x = random() % (SCREEN_WIDTH - 32);
    bell->y = -32 - (random() % 100);
    bell->isBlinking = FALSE;
    bell->blinkCounter = 0;
    SPR_setVisibility(bell->sprite, VISIBLE);
    SPR_setPosition(bell->sprite, bell->x, bell->y);
}

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

/* ═══════════════════════════════════════════════════════════════════════════
   FUNCIONES - CAMPANILLAS FIJAS
   ═══════════════════════════════════════════════════════════════════════════ */

static void initFixedBells(void) {
    s16 separador_x = 24, separador_y = 16;
    s16 x = 0;
    s16 y = SCREEN_HEIGHT - 3 * separador_y - 16;
    
    for (u8 i = 0; i < NUM_FIXED_BELLS; i++) {
        fixedBells[i].sprite = SPR_addSprite(&sprite_campana_bn, x, y,
            TILE_ATTR(PAL_PLAYER, FALSE, FALSE, FALSE));
        fixedBells[i].active = FALSE;
        
        x += separador_x;
        if (i == 3 || i == 7) {
            y += separador_y;
            x = 0;
        }
    }
}

static void updateFixedBells(void) {
    if (bellsCompleted < NUM_FIXED_BELLS) {
        u8 usarColor = (frameCounter % 4) < 2;
        SPR_setDefinition(fixedBells[bellsCompleted].sprite,
            usarColor ? &sprite_campana : &sprite_campana_bn);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
   FUNCIONES - BOMBAS
   ═══════════════════════════════════════════════════════════════════════════ */

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

static void resetBomb(Bomb* bomb) {
    bomb->x = random() % (SCREEN_WIDTH - 32);
    bomb->y = -32;
    bomb->isBlinking = FALSE;
    bomb->blinkCounter = 0;
    SPR_setVisibility(bomb->sprite, VISIBLE);
    SPR_setPosition(bomb->sprite, bomb->x, bomb->y);
}

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

/* ═══════════════════════════════════════════════════════════════════════════
   FUNCIONES - BALAS/CONFETI
   ═══════════════════════════════════════════════════════════════════════════ */

static void initBullets(void) {
    for (u8 i = 0; i < NUM_BULLETS; i++) {
        bullets[i].sprite = NULL;
        bullets[i].active = FALSE;
        bullets[i].x = 0;
        bullets[i].y = 0;
    }
    activeBullets = 0;
}

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

static void desactivarBala(Bullet* bala) {
    if (bala->active) {
        SPR_releaseSprite(bala->sprite);
        bala->sprite = NULL;
        bala->active = FALSE;
        activeBullets--;
    }
}

static void detectarColisionesBala(Bullet* bala) {
    s16 balaCentroX = bala->x + 4;
    s16 balaCentroY = bala->y + 4;
    
    /* Colisiones con campanas */
    for (u8 i = 0; i < NUM_BELLS; i++) {
        if (bells[i].isBlinking) continue;
        
        if (balaCentroX >= bells[i].x &&
            balaCentroX < bells[i].x + 32 &&
            balaCentroY >= bells[i].y + 6 &&
            balaCentroY < bells[i].y + 26) {
            
            XGM2_playPCM(snd_campana, sizeof(snd_campana), SOUND_PCM_CH2);
            desactivarBala(bala);
            
            bells[i].isBlinking = TRUE;
            bells[i].blinkCounter = FRAMES_BLINK;
            
            if (bellsCompleted < NUM_FIXED_BELLS) {
                SPR_setDefinition(fixedBells[bellsCompleted].sprite, &sprite_campana);
                fixedBells[bellsCompleted].active = TRUE;
                bellsCompleted++;
            }
            return;
        }
    }
    
    /* Colisiones con bombas */
    for (u8 i = 0; i < NUM_BOMBS; i++) {
        if (bombs[i].isBlinking) continue;
        
        if (balaCentroX >= bombs[i].x &&
            balaCentroX < bombs[i].x + 32 &&
            balaCentroY >= bombs[i].y + 6 &&
            balaCentroY < bombs[i].y + 26) {
            
            XGM2_playPCM(snd_bomba, sizeof(snd_bomba), SOUND_PCM_CH_AUTO);
            desactivarBala(bala);
            
            for (u8 j = 0; j < NUM_BOMBS; j++) {
                bombs[j].isBlinking = TRUE;
                bombs[j].blinkCounter = FRAMES_BLINK;
            }
            
            for (u8 j = 0; j < NUM_BELLS; j++) {
                bells[j].isBlinking = TRUE;
                bells[j].blinkCounter = FRAMES_BLINK;
            }
            
            bellsCompleted = 0;
            for (u8 j = 0; j < NUM_FIXED_BELLS; j++) {
                SPR_setDefinition(fixedBells[j].sprite, &sprite_campana_bn);
            }
            return;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
   API PÚBLICA
   ═══════════════════════════════════════════════════════════════════════════ */

void minigameBells_init(void) {
    VDP_setScreenWidth320();
    VDP_setScreenHeight224();
    
    SPR_init();
    JOY_init();
    
    if (sprite_campana.palette) {
        PAL_setPalette(PAL_PLAYER, sprite_campana.palette->data, CPU);
    }
    if (sprite_bomba.palette) {
        PAL_setPalette(PAL_EFFECT, sprite_bomba.palette->data, CPU);
    }
    if (image_fondo_pal.data) {
        PAL_setPalette(PAL_COMMON, image_fondo_pal.data, CPU);
    }
    
    VDP_setBackgroundColor(0);
    
    /* Cargar fondo */
    tileIndex = TILE_USER_INDEX;
    
    VDP_loadTileSet(&image_fondo_tile, tileIndex, CPU);
    mapBackground = MAP_create(&image_fondo_map, BG_B,
        TILE_ATTR_FULL(PAL_COMMON, FALSE, FALSE, FALSE, tileIndex));
    tileIndex += image_fondo_tile.numTile;
    MAP_scrollTo(mapBackground, 0, 0);
    
    VDP_loadTileSet(&image_fondo_nieve_tile, tileIndex, CPU);
    mapSnow = MAP_create(&image_fondo_nieve_map, BG_A,
        TILE_ATTR_FULL(PAL_COMMON, TRUE, FALSE, FALSE, tileIndex));
    tileIndex += image_fondo_nieve_tile.numTile;
    MAP_scrollTo(mapSnow, 0, 0);
    
    /* Música */
    XGM2_setLoopNumber(-1);
    XGM2_play(musica_fondo);
    XGM2_setFMVolume(70);
    XGM2_setPSGVolume(100);
    
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
    gameCore_initTimer(&gameTimer, 999);
    
    /* Variables */
    bellsCompleted = 0;
    frameCounter = 0;
    cannonVelocity = 0;
    cannonFiring = FALSE;
    bulletCooldown = 0;
    snowOffsetX = 0;
    snowOffsetY = 0;
}

void minigameBells_update(void) {
    u16 input = gameCore_readInput();
    
    /* Cañón */
    if (input & BUTTON_LEFT) {
        cannonVelocity = (cannonVelocity > -CANNON_MAX_VEL) ?
            cannonVelocity - CANNON_ACCEL : -CANNON_MAX_VEL;
    } else if (input & BUTTON_RIGHT) {
        cannonVelocity = (cannonVelocity < CANNON_MAX_VEL) ?
            cannonVelocity + CANNON_ACCEL : CANNON_MAX_VEL;
    } else {
        if (cannonVelocity > 0) {
            cannonVelocity -= CANNON_FRICTION;
        } else if (cannonVelocity < 0) {
            cannonVelocity += CANNON_FRICTION;
        }
    }
    
    cannonX += cannonVelocity;
    if (cannonX < -32) cannonX = -32;
    if (cannonX > SCREEN_WIDTH - 32) cannonX = SCREEN_WIDTH - 32;
    SPR_setPosition(playerCannon, cannonX, SCREEN_HEIGHT - 64);
    
    /* Disparos */
    if ((input & BUTTON_A) && bulletCooldown <= 0) {
        cannonFiring = TRUE;
        SPR_setAnim(playerCannon, 1);
        fireBullet();
        bulletCooldown = BULLET_COOLDOWN_FRAMES;
    }
    
    if (cannonFiring) {
        if (SPR_isAnimationDone(playerCannon)) {
            cannonFiring = FALSE;
            SPR_setAnim(playerCannon, 0);
        }
    }
    
    /* Paralaje */
    snowOffsetX = sinFix16(frameCounter * 2);
    snowOffsetY--;
    MAP_scrollTo(mapSnow, snowOffsetX, snowOffsetY);
    
    /* Actualizar objetos */
    for (u8 i = 0; i < NUM_BELLS; i++) {
        updateBell(&bells[i]);
    }
    
    updateFixedBells();
    
    for (u8 i = 0; i < NUM_BOMBS; i++) {
        updateBomb(&bombs[i]);
    }
    
    updateBullets();
    
    /* Timer y contadores */
    gameCore_updateTimer(&gameTimer);
    frameCounter++;
    if (bulletCooldown > 0) bulletCooldown--;
}

void minigameBells_render(void) {
    SPR_update();
    SYS_doVBlankProcess();
}

u8 minigameBells_isComplete(void) {
    return (bellsCompleted >= NUM_FIXED_BELLS);
}