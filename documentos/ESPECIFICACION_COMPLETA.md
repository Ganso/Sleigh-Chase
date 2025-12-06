# 🎄 ESPECIFICACIÓN TÉCNICA COMPLETA - FELIZ 2026

**Proyecto Navideño Sega Mega Drive - Especificación Exhaustiva y Metódica**

---

## 📊 TABLA DE CONTENIDOS

1. [Visión General del Proyecto](#visión-general)
2. [Arquitectura de Fases](#arquitectura-de-fases)
3. [Especificaciones Técnicas Generales](#especificaciones-técnicas-generales)
4. [Fase 1 - Recogida (Polo Norte)](#fase-1)
5. [Fase 2 - Entrega (Tejados)](#fase-2)
6. [Fase 3 - Campanadas (Iglesia)](#fase-3)
7. [Fase 4 - Celebración (Fiesta)](#fase-4)
8. [Estructura de Archivos](#estructura-de-archivos)
9. [Assets Gráficos (GFX)](#assets-gráficos)
10. [Assets de Audio (SFX/Música)](#assets-de-audio)
11. [Requisitos de Memoria](#requisitos-de-memoria)
12. [Máquina de Estados](#máquina-de-estados)

---

## <a name="visión-general"></a>

## 1️⃣ VISIÓN GENERAL DEL PROYECTO

### Concepto
- **Nombre**: FELIZ 2026 (Navidad para Sega Mega Drive)
- **Género**: Colección de 4 minijuegos navideños
- **Plataforma**: Sega Genesis/Mega Drive (original)
- **Resolución**: 320×224 píxeles
- **Paletas**: 4 independientes (PAL_COMMON, PAL_PLAYER, PAL_EFFECT, PAL_BACKGROUND)
- **RAM Disponible**: 64KB (máximo SGDK puede usar ~50KB)
- **FPS**: 60 FPS constantes
- **Lenguaje**: C + SGDK

### Audiencia
- Nostálgica de juegos retro
- Apreciadores de Mega Drive
- Audiencia familiar navideña

### Narrativa
Cuatro fases que simulan las tradiciones navideñas:
1. Recogida de regalos en Polo Norte
2. Entrega de regalos en tejados
3. Campanadas de celebración en iglesia
4. Fiesta final de celebración

---

## <a name="arquitectura-de-fases"></a>

## 2️⃣ ARQUITECTURA DE FASES

```
FLUJO PRINCIPAL: main.c → Orquestador

    │
    ├─→ Logo Intro (3-5 segundos)
    │    └─ Video: Splash screen Geesebumps/FELIZ
    │
    ├─→ FASE 1: Recogida (Polo Norte)
    │    ├─ Duración: 60-90 segundos
    │    ├─ Archivo: minigame_pickup.c
    │    ├─ Ubicación: Polo Norte (nieve)
    │    ├─ Objetivo: Recoger 20 regalos
    │    └─ Victoria: Transición a Fase 2
    │
    ├─→ FASE 2: Entrega (Tejados)
    │    ├─ Duración: 60-90 segundos
    │    ├─ Archivo: minigame_delivery.c
    │    ├─ Ubicación: Tejados nocturnos
    │    ├─ Objetivo: Entregar 10 regalos en chimeneas
    │    └─ Victoria: Transición a Fase 3
    │
    ├─→ FASE 3: Campanadas (Iglesia)
    │    ├─ Duración: Ilimitado (mecánica de arcade)
    │    ├─ Archivo: minigame_bells.c ✅ YA IMPLEMENTADA
    │    ├─ Ubicación: Iglesia (interior)
    │    ├─ Objetivo: Tocar 12 campanillas
    │    └─ Victoria: Transición a Fase 4
    │
    ├─→ FASE 4: Celebración (Fiesta)
    │    ├─ Duración: 15-30 segundos
    │    ├─ Archivo: minigame_celebration.c
    │    ├─ Ubicación: Salón de fiesta
    │    ├─ Objetivo: Animación de victoria
    │    └─ Victoria: Fin del juego
    │
    └─→ Pantalla Final: Mensaje "¡FELIZ 2026!"
         └─ Presionar botón para salir
```

---

## <a name="especificaciones-técnicas-generales"></a>

## 3️⃣ ESPECIFICACIONES TÉCNICAS GENERALES

### Hardware Constraints
```
│ Aspecto              │ Valor          │ Limitación            │
│─────────────────────│────────────────│──────────────────────│
│ RAM Total           │ 64KB           │ Máximo por hardware   │
│ VRAM                │ 64KB           │ Video memory          │
│ Sprites Máx         │ 80             │ Render priority       │
│ Sprites por línea   │ 16             │ Mega Drive hardware   │
│ Tiles en pantalla   │ 320×224 = 40×28│ Aprox 1120 tiles      │
│ FPS                 │ 60             │ NTSC (50 PAL)         │
│ Paletas             │ 4 × 16 colores │ 64 colores disponibles│
│ Audio Channels      │ 2 + PCM        │ FM synth + PCM        │
```

### Resolución y Offsets
```
PANTALLA: 320×224

   ┌─────────────────────────────────────────┐
   │  (0,0)                          (319,0) │
   │                                         │
   │                                         │
   │  Área de juego 320×224                  │
   │                                         │
   │                                         │
   │  (0,223)                      (319,223)│
   └─────────────────────────────────────────┘

ZONA HUD:
- Esquina superior derecha: FELIZ (30×30 píxeles aprox)
- Fondo: Zona inferior (para contador/progreso)
- Disponible: Máximo 224 píxeles de alto
```

### Paletas de Color
```
PALETA 0 (PAL_COMMON):
├─ Color 0: Negro (fondo)
├─ Colores 1-15: Fondos y decoración
└─ Ejemplo: Nieve (blanco), noche (azul oscuro)

PALETA 1 (PAL_PLAYER):
├─ Color 0: Transparente (magenta)
├─ Colores 1-15: Sprites del cañón y campanillas
└─ Rango: Amarillo dorado, rojo, marrón

PALETA 2 (PAL_EFFECT):
├─ Color 0: Transparente (magenta)
├─ Colores 1-15: Efectos (bombas, confeti, etc)
└─ Rango: Rojo vivo, violeta, naranja

PALETA 3 (PAL_BACKGROUND):
├─ Color 0: Transparente (magenta)
├─ Colores 1-15: Decoración segunda capa
└─ Rango: Azul cielo, verde árbol, etc
```

### Tipos de Datos SGDK
```
Sprites:
  SpriteDefinition sprite_x     // Definición del sprite
  Sprite* ptr_sprite            // Puntero a sprite en pantalla

Maps/Fondos:
  TileSet tileSet               // Definición de tiles
  MapDefinition mapDef          // Definición del mapa
  Map* map                      // Mapa cargado

Paletas:
  Palette paleta                // 16 colores

Audio:
  u8 soundData[SIZE]            // PCM sin comprimir
  XGM2_play(musicData)          // Reproducir música VGM
```

---

## <a name="fase-1"></a>

## 4️⃣ FASE 1 - RECOGIDA (POLO NORTE)

### Descripción General
- **Escenario**: Pista de nieve cenital con scroll vertical continuo en bucle de dos pantallas.
- **Duración**: 60-90 segundos aproximados.
- **Mecánica**: Conducir el carro de Papá Noel evitando laterales y duendes, recogiendo regalos de árboles y elfos.
- **Disparos**: No hay disparo; el único poder ofensivo es la limpieza de pantalla cuando la nariz de Rudolf parpadea.

### Gameplay

**Objetivo Principal**
```
Recoger 15 REGALOS para pasar a Fase 2.

┌─ Fuentes de regalo:
│  ├─ Tocar árboles (64×64) colocados sobre la pista.
│  └─ Recibir lanzamientos de elfos aliados situados en los laterales.
└─ Victoria: 15 regalos acumulados.
```

**Mecánica de Juego**
```
ENTRADA:
├─ LEFT/RIGHT → Desplazar el carro dentro de la pista (sin invadir laterales).
├─ B          → Activar poder especial si la nariz parpadea (limpia duendes).
└─ START      → Pausa (opcional).

ESCENARIO:
├─ Scroll vertical ascendente (MAP image_pista_polo_map) de 320×448 px en bucle.
├─ Laterales prohibidos: 20% de la pantalla a cada lado con vallas dibujadas.
└─ Objetos de pista bajan al mismo ritmo que el scroll para simular estáticos.

OBJETOS ALIADOS:
├─ Árboles 64×64 (sprite_arbol_pista) que al tocarlos suman 1 regalo.
├─ Elfos 32×32 en los laterales (sprite_elfo_lateral) que entregan 1 regalo al acercarse.
└─ Carro de Papá Noel 80×128 (sprite_santa_car) con animación de nariz normal/parpadeo.

ENEMIGOS:
├─ Duendes malvados 32×32 (sprite_duende_malo) que caen con el scroll y se mueven lateralmente hacia el carro.
├─ Colisión con un duende resta 1 regalo (mínimo 0) y reubica al enemigo.
└─ Poder especial: disponible cada 3 regalos recogidos; elimina todos los duendes en pantalla.

SCORE/PROGRESO:
├─ HUD: "Regalos: X/15" y "Especial: cargo/3".
├─ La nariz parpadea al alcanzar la carga completa del poder especial.
└─ Completar objetivo → transición a Fase 2.
```

### Recursos

**Sprites principales**
- `sprite_santa_car` (80×128, 2 frames) – Carro visto desde arriba, segundo frame con nariz parpadeando.
- `sprite_arbol_pista` (64×64, 1 frame) – Árbol nevado sobre la pista.
- `sprite_elfo_lateral` (32×32, 1 frame) – Elfo que entrega regalos desde el borde.
- `sprite_duende_malo` (32×32, 1 frame) – Duende enemigo que intenta chocar.
- `sprite_regalo` (32×32, 1 frame) – Regalo usado en contadores y lanzamientos.

**Fondo y mapa**
- `image_pista_polo_tile` + `image_pista_polo_map` (320×448, 40×56 tiles) – Pista vertical en bucle con vallas laterales; usa `image_pista_polo_pal`.

**Sonidos reutilizados**
- `snd_regalo_recogido`: confirma recolección de regalos (árbol o elfo).
- `snd_obstaculo_golpe`: puede acompañar colisión con duendes si se desea feedback.
## <a name="fase-2"></a>

## 5️⃣ FASE 2 - ENTREGA (TEJADOS)

### Descripción General
- **Escenario**: Tejados de ciudad (noche)
- **Duración**: 60-90 segundos
- **Dificultad**: Media
- **Mecánica**: Lanzar regalos a chimeneas

### Gameplay

**Objetivo Principal**
```
Entregar 10 REGALOS EN CHIMENEAS

┌─ Requisitos:
│  ├─ 10 entregas exitosas
│  ├─ Chimeneas objetivo: 15 (distribuidas)
│  ├─ Regalos: 20 total (no limitado por fase anterior)
│  └─ Fallos permitidos: Ilimitados
└─ Victoria: 10/10 entregas completadas
```

**Mecánica de Juego**
```
ENTRADA:
├─ LEFT/RIGHT    → Mover cañón
├─ A             → Disparar regalo
├─ B             → Contacto directo
└─ START         → Pausa (opcional)

FÍSICAS:
├─ Regalos objetivo: Caen desde arriba → entrar en chimeneas
├─ Cañón: Lanza regalos hacia arriba/diagonales
├─ Gravedad: Afecta a regalos en movimiento
└─ Chimeneas: Fijas en pantalla (20 píxeles ancho)

COLISIONES:
├─ Regalo en chimenea = ENTREGA +1
├─ Regalo fuera = Se pierde sin penalización
├─ Regalo + obstáculo = Se detiene
└─ Límite de entregas: No hay límite de intentos

SCORE/PROGRESO:
├─ HUD: Muestra "Entregas: X/10"
├─ Visualización: Chimeneas iluminadas = completadas
└─ Al 100% → "¡VICTORIA!" + transición
```

**Dificultad Progression**
```
Tiempo 0-30s:   3 chimeneas activas
Tiempo 30-60s:  5 chimeneas activas
Tiempo 60-90s:  7 chimeneas activas (máximo)

Movimiento:
├─ Cañón: velocidad constante
├─ Chimeneas: Pueden moverse lentamente (-1 a +1 píx/s)
└─ Viento: Afecta trayectoria (+/- 0.5 píxeles/frame)
```

### Sprites Requeridos

**sprite_regalo_entrega**
```
Tamaño: 24×24 píxeles
Frames: 1
Colores: Rojo + cinta dorada
Paleta: PAL_PLAYER
Archivo: Regalos_Pequeño.png (24×24)
```

**sprite_chimenea**
```
Tamaño: 20×40 píxeles (altura del tejado)
Frames: 2 (apagada, encendida)
Colores: Ladrillo rojo, gris humo
Paleta: PAL_PLAYER
Archivo: Chimenea.png (20×80)
```

**sprite_canon_tejado**
```
Tamaño: 64×48 píxeles
Frames: 2 (reposo, disparo)
Colores: Madera, detalles metálicos
Paleta: PAL_PLAYER
Archivo: CanonTejado.png (64×96)
```

**sprite_nube**
```
Tamaño: 64×32 píxeles
Frames: 1 (paralaje)
Colores: Blanco, gris suave
Paleta: PAL_COMMON
Archivo: Nube.png (64×32)
```

### Fondos

**image_fondo_tejados**
```
Tamaño: 320×224 píxeles
Tipo: Tileset + Map
Contenido: Tejados, chimeneas, luna llena
Paleta: PAL_COMMON (colores nocturnos: azul, gris)
Archivo Gráfico: FondoTejados.png (512×256 min)
Parallax: NO (fondo base)
```

**image_nubes_paralaje**
```
Tamaño: 320×224 píxeles
Tipo: Tileset paralaje
Contenido: Nubes flotantes
Paleta: PAL_COMMON
Archivo: Nubes.png (256×256)
Parallax: Sí (scroll: -0.5 píxeles/frame)
```

### Sonidos (SFX)

**snd_regalo_disparado**
```
Duración: ~0.15 segundos
Tipo: Acción suave
Frecuencia: 8kHz, 8-bit PCM
Tamaño: ~1200 bytes
Contenido: "Whomp" de lanzamiento
Reproducir: SOUND_PCM_CH_AUTO
```

**snd_entrega_exitosa**
```
Duración: ~0.3 segundos
Tipo: Sonido positivo
Frecuencia: 8kHz, 8-bit PCM
Tamaño: ~2400 bytes
Contenido: "Ding-dong" campana
Reproducir: SOUND_PCM_CH1
```

**snd_chimenea_activa**
```
Duración: ~0.2 segundos
Tipo: Sonido de encendido
Frecuencia: 8kHz, 8-bit PCM
Tamaño: ~1600 bytes
Contenido: "Pop" de fuego
Reproducir: SOUND_PCM_CH2
```

### Música

**musica_tejados**
```
Tipo: XGM2 (VGM format)
Tempo: 100 BPM (más lento, ambiental)
Duración: 30-60 segundos loop
Instrumento principal: Sintetizador FM
Estilo: Navideño nocturno, misterioso
Tamaño: ~3-5 KB
Loop: Sí
Volumen FM: 60
Volumen PSG: 90
Reproducir: XGM2_play(musica_tejados) con loop
```

### Variables Locales (minigame_delivery.c)

```c
/* Estructuras */
typedef struct {
    Sprite* sprite;
    s16 x, y;
    u8 active;
} Chimenea;

typedef struct {
    Sprite* sprite;
    s16 x, y;
    s16 velX, velY;
    u8 active;
} Regalo_Volador;

/* Arrays */
static Chimenea chimeneas[NUM_CHIMENEAS];        // 15 distribuidas
static Regalo_Volador regalos[NUM_REGALOS_VOL]; // 6 en vuelo
static Sprite* cannonTejado;

/* Globales */
static s16 cannonX;
static s8 cannonVelocity;
static u16 entregasCompletadas;                 // 0-10
static u16 frameCounter;

/* Constantes */
#define NUM_CHIMENEAS 15
#define NUM_REGALOS_VOL 6
#define OBJETIVO_ENTREGAS 10
```

---

## <a name="fase-3"></a>

## 6️⃣ FASE 3 - CAMPANADAS (IGLESIA) ✅

### ESTADO: 100% IMPLEMENTADA

**Ver archivo**: `minigame_bells.c` (FINAL)

### Resumen Ejecutivo
```
FASE 3: Campanadas (Iglesia)
├─ Estado: ✅ COMPLETA
├─ Duración: Ilimitada (arcade)
├─ Objetivo: 12 campanillas
├─ Dificultad: Media-Alta
├─ Sistema: Disparos de confeti
└─ Archivo: minigame_bells.c
```

### Especificaciones (Resumen)

**Campanas Cayendo**: 6 en pantalla
```
├─ Tamaño: 32×32 píxeles
├─ Velocidad: Variable 1-3 píxeles/frame
├─ Reinicio: Automático cuando toca piso
└─ Parpadeo: Al ser tocadas (FRAMES_BLINK = 30)
```

**Campanillas HUD**: 12 objetivo
```
├─ Disposición: Grid 4×3 en base pantalla
├─ Tamaño: 24×24 píxeles c/u
├─ Color: Gris hasta completarse
├─ Animación: Parpadea cuando se completa
└─ Contador: Visual + lógico
```

**Bombas Enemigas**: 3 en pantalla
```
├─ Tamaño: 32×32 píxeles
├─ Efecto: Reinicia contador si se tocan
├─ Velocidad: Variable 1-4 píxeles/frame
└─ Animación: Parpadea cuando se tocan
```

**Balas/Confeti**: 3 máximo
```
├─ Tamaño: 8×8 píxeles
├─ Velocidad: 2 píxeles/frame (subida)
├─ Cooldown: 30 frames entre disparos
└─ Física: Movimiento recto vertical
```

**Cañón**: 1 sprite
```
├─ Tamaño: 80×64 píxeles
├─ Frames: 2 (reposo, disparo)
├─ Movimiento: LEFT/RIGHT con aceleración
├─ Vel máx: 6 píxeles/frame
└─ Control: Suave con fricción
```

**Fondos**: 2 layers
```
Capa 1 (BGB - fondo):
├─ Iglesia interior
├─ Paralaje: NO
└─ Paleta: PAL_COMMON

Capa 2 (BGA - paralaje):
├─ Nieve animada
├─ Paralaje: Scroll diagonal lento
└─ Paleta: PAL_COMMON
```

**Audio**:
```
Música: musica_fondo (XGM2 loop)
SFX Campana: snd_campana (25KB)
SFX Bomba: snd_bomba (10KB)
SFX Cañón: snd_canon (5KB)
```

---

## <a name="fase-4"></a>

## 7️⃣ FASE 4 - CELEBRACIÓN (FIESTA)

### Descripción General
- **Escenario**: Salón de fiesta decorado
- **Duración**: 15-30 segundos
- **Dificultad**: Ninguna (animación)
- **Mecánica**: Secuencia de victoria

### Gameplay

**Objetivo Principal**
```
Mostrar animación de VICTORIA

┌─ Requisitos:
│  ├─ Animar confeti cayendo
│  ├─ Mostrar mensaje "¡FELIZ 2026!"
│  ├─ Reproducir música de celebración
│  └─ Permitir continuar/salir
└─ Duración: 15-30 segundos auto-advance
```

**Secuencia**
```
FRAME 0-30 (Fade in):
├─ Fundido desde negro
├─ Fade música celebración (de 0 a 100%)
└─ Mostrar "¡VICTORIA!"

FRAME 30-180 (Celebración):
├─ Confeti animado cayendo
├─ Sprites navideños danzando
├─ Luces parpadeantes
└─ Aplausos de fondo (SFX loop)

FRAME 180-210 (Fade out):
├─ Parpadeo final
├─ "Presiona START para continuar"
├─ Fade música (de 100% a 0)
└─ Esperar entrada

FRAME 210+:
├─ Transición a pantalla final
└─ Mostrar créditos/fin
```

### Sprites Requeridos

**sprite_confeti**
```
Tamaño: 4×4 píxeles
Frames: 1
Colores: Multicolor aleatorio
Paleta: PAL_EFFECT
Archivo: Confeti.png (12×12, 3×3 grid)
Cantidad: 30-50 sprites en pantalla
```

**sprite_personaje_feliz**
```
Tamaño: 64×64 píxeles
Frames: 4 (animación baile)
Colores: Ropa colorida navideña
Paleta: PAL_PLAYER
Archivo: PersonajeFilesta.png (256×64)
```

**sprite_árbol_fiesta**
```
Tamaño: 80×120 píxeles
Frames: 2 (luces encendidas/apagadas)
Colores: Verde árbol + luces multicolor
Paleta: PAL_PLAYER
Archivo: ÁrbolFiesta.png (80×240)
```

**sprite_globo**
```
Tamaño: 24×32 píxeles
Frames: 1
Colores: Rojo, azul, amarillo
Paleta: PAL_EFFECT
Archivo: Globo.png (72×32, 3 colores)
```

### Fondos

**image_fondo_fiesta**
```
Tamaño: 320×224 píxeles
Tipo: Tileset + Map
Contenido: Salón decorado, guirnaldas, luces
Paleta: PAL_COMMON (colores cálidos: rojo, oro)
Archivo Gráfico: FondoFiesta.png (512×256 min)
Parallax: NO (fondo base)
```

**image_luces_parpadeo**
```
Tamaño: 320×224 píxeles
Tipo: Tileset animado
Contenido: Luces parpadeantes
Paleta: PAL_EFFECT
Archivo: LucesParpadeo.png (256×256)
Parallax: NO (overlay)
Animación: Parpadea cada 10 frames
```

### Sonidos (SFX)

**snd_victoria**
```
Duración: ~2 segundos
Tipo: Fanfarra triunfal
Frecuencia: 8kHz, 8-bit PCM
Tamaño: ~16KB
Contenido: Música de victoria
Reproducir: Automático al iniciar fase
```

**snd_aplausos**
```
Duración: ~3 segundos
Tipo: Ambiente
Frecuencia: 8kHz, 8-bit PCM
Tamaño: ~24KB
Contenido: Multitud aplaudiendo
Reproducir: SOUND_PCM_CH_AUTO (loop 2x)
```

**snd_choque_confeti**
```
Duración: ~0.1 segundos
Tipo: Efecto (opcional)
Frecuencia: 8kHz, 8-bit PCM
Tamaño: ~800 bytes
Contenido: Sonido suave de confeti
Reproducir: Cada 2-3 frames de confeti
```

### Música

**musica_celebracion**
```
Tipo: XGM2 (VGM format)
Tempo: 140 BPM (rápido, festivo)
Duración: 30-60 segundos loop
Instrumento principal: Sintetizador cheerful
Estilo: Navideño upbeat, festivo
Tamaño: ~4-6 KB
Loop: Sí (cuenta como victoria)
Volumen FM: 80
Volumen PSG: 110 (máximo)
Reproducir: XGM2_play(musica_celebracion)
```

### Variables Locales (minigame_celebration.c)

```c
/* Estructuras */
typedef struct {
    Sprite* sprite;
    s16 x, y;
    s8 velX, velY;
    u8 active;
    u8 rotation;
} Confeti;

typedef struct {
    Sprite* sprite;
    s16 x, y;
    u8 frameAnim;
} PersonajeFiesta;

/* Arrays */
static Confeti confetis[NUM_CONFETI];        // 40-50 máximo
static PersonajeFiesta personajes[NUM_PERSONAJES]; // 1-2

/* Globales */
static u16 frameCounter;
static u8 secuenciaPhase;                    // 0=fade in, 1=celebra, 2=fade out
static GameTimer celebrationTimer;

/* Constantes */
#define NUM_CONFETI 50
#define NUM_PERSONAJES 2
#define DURACION_CELEBRACION 300  // 5 segundos a 60 FPS
```

---

## <a name="estructura-de-archivos"></a>

## 8️⃣ ESTRUCTURA DE ARCHIVOS

```
christmas-game-2026/
│
├─ src/
│  ├─ main.c                      ✅ Orquestador principal
│  ├─ game_core.c                 ✅ Funciones reutilizables
│  ├─ geesebumps.c                ✅ Intro logo
│  ├─ minigame_bells.c            ✅ FASE 3 (COMPLETA)
│  ├─ minigame_pickup.c           📋 FASE 1 (TODO)
│  ├─ minigame_delivery.c         📋 FASE 2 (TODO)
│  ├─ minigame_celebration.c      📋 FASE 4 (TODO)
│  ├─ boot/
│  │  ├─ romhead.c                ✅ ROM header
│  │  └─ sega.s                   ✅ Startup asm
│  └─ res/
│     ├─ resources.h              🔄 Será generado
│     └─ resources.res            🔄 Será generado
│
├─ inc/
│  ├─ game_core.h                 ✅ Headers core
│  ├─ geesebumps.h                ✅ Headers intro
│  ├─ minigame_bells.h            ✅ Headers Fase 3
│  ├─ minigame_pickup.h           📋 Headers Fase 1
│  ├─ minigame_delivery.h         📋 Headers Fase 2
│  └─ minigame_celebration.h      📋 Headers Fase 4
│
├─ res/
│  ├─ sprites/               (Campana*, Canon, Bomba, Confeti; TODO resto fases)
│  ├─ bg/                    (Fondo.png, FondoNieve.png; TODO fondos Polo/Tejados/Fiesta)
│  ├─ sfx/                   (snd_campana, snd_bomba, snd_canon; TODO más SFX)
│  ├─ music/                 (musica.vgm fase 3; TODO músicas fases 1/2/4)
│  ├─ Geesebumps/            (paletas + logos + Goosebumps_intro.vgm)
│  ├─ resources.h            (generado por rescomp)
│  └─ resources.res          (directivas SGDK)
│
├─ build/
│  └─ rom.bin                    (Salida final)
│
├─ Makefile                       ✅
├─ README.md                      ✅
└─ ...
```

---

## <a name="assets-gráficos"></a>

## 9️⃣ ASSETS GRÁFICOS (GFX)

### Resumen de Sprites

```
┌─ CATEGORÍA      │ SPRITE              │ TAMAÑO    │ FRAMES │ PALETA      │ ESTADO  │
├──────────────────┼────────────────────┼──────────┼────────┼─────────────┼─────────┤
│ FASE 3           │                    │          │        │             │         │
├──────────────────┼────────────────────┼──────────┼────────┼─────────────┼─────────┤
│ Campanillas      │ sprite_campana     │ 32×32    │ 1      │ PAL_PLAYER  │ ✅      │
│                  │ sprite_campana_bn  │ 32×32    │ 1      │ PAL_PLAYER  │ ✅      │
├──────────────────┼────────────────────┼──────────┼────────┼─────────────┼─────────┤
│ Cañón            │ sprite_canon       │ 80×64    │ 2      │ PAL_PLAYER  │ ✅      │
├──────────────────┼────────────────────┼──────────┼────────┼─────────────┼─────────┤
│ Efectos          │ sprite_bomba       │ 32×32    │ 1      │ PAL_EFFECT  │ ✅      │
│                  │ sprite_bola_confeti│ 8×8      │ 1      │ PAL_EFFECT  │ ✅      │
├──────────────────┼────────────────────┼──────────┼────────┼─────────────┼─────────┤
│ FASE 1 (TODO)    │                    │          │        │             │         │
├──────────────────┼────────────────────┼──────────┼────────┼─────────────┼─────────┤
│ Regalos          │ sprite_regalo      │ 32×32    │ 1      │ PAL_PLAYER  │ 📋      │
│                  │ sprite_regalo_bn   │ 32×32    │ 1      │ PAL_PLAYER  │ 📋      │
├──────────────────┼────────────────────┼──────────┼────────┼─────────────┼─────────┤
│ Cañón Polo       │ sprite_canon_polo  │ 80×64    │ 2      │ PAL_PLAYER  │ 📋      │
├──────────────────┼────────────────────┼──────────┼────────┼─────────────┼─────────┤
│ Proyectiles      │ sprite_red_confeti │ 8×8      │ 1      │ PAL_EFFECT  │ 📋      │
├──────────────────┼────────────────────┼──────────┼────────┼─────────────┼─────────┤
│ Obstáculos       │ sprite_copo_grande │ 64×64    │ 1      │ PAL_COMMON  │ 📋      │
├──────────────────┼────────────────────┼──────────┼────────┼─────────────┼─────────┤
│ FASE 2 (TODO)    │                    │          │        │             │         │
├──────────────────┼────────────────────┼──────────┼────────┼─────────────┼─────────┤
│ Chimeneas        │ sprite_chimenea    │ 20×80    │ 2      │ PAL_PLAYER  │ 📋      │
├──────────────────┼────────────────────┼──────────┼────────┼─────────────┼─────────┤
│ Cañón Tejado     │ sprite_canon_tejado│ 64×96    │ 2      │ PAL_PLAYER  │ 📋      │
├──────────────────┼────────────────────┼──────────┼────────┼─────────────┼─────────┤
│ Decoración       │ sprite_nube        │ 64×32    │ 1      │ PAL_COMMON  │ 📋      │
├──────────────────┼────────────────────┼──────────┼────────┼─────────────┼─────────┤
│ FASE 4 (TODO)    │                    │          │        │             │         │
├──────────────────┼────────────────────┼──────────┼────────┼─────────────┼─────────┤
│ Confeti          │ sprite_confeti     │ 4×4      │ 1      │ PAL_EFFECT  │ 📋      │
├──────────────────┼────────────────────┼──────────┼────────┼─────────────┼─────────┤
│ Personajes       │ sprite_personaje   │ 64×64    │ 4      │ PAL_PLAYER  │ 📋      │
├──────────────────┼────────────────────┼──────────┼────────┼─────────────┼─────────┤
│ Decoración       │ sprite_arbol_fiesta│ 80×120   │ 2      │ PAL_PLAYER  │ 📋      │
│                  │ sprite_globo       │ 24×32    │ 1      │ PAL_EFFECT  │ 📋      │
└──────────────────┴────────────────────┴──────────┴────────┴─────────────┴─────────┘
```

### Requisitos Técnicos de Sprites

**Formato PNG**:
- Transparencia: Magenta (#FF00FF) = transparente
- Indexado: 256 colores máximo
- Optimización: Reducir a 16 colores si es posible
- Compresión: PNG-9 para tamaño mínimo

**Tamaño SGDK**:
- Todos los sprites deben ser múltiplos de 8 píxeles
- Máximo: 1024×1024 en textura
- Típico: 32×32, 64×64, 80×64, etc

**Animación**:
- Frames horizontales (left-to-right)
- Ejemplo sprite_canon (2 frames): 160×64 → 80×64 c/frame
- SPR_ANIM(0, 1, 2, 0) → frame 0 y 1, 2 velocidad

### Requisitos Técnicos de Fondos

**Tileset + Map**:
- TileSet: Imagen dividida en tiles 8×8
- MapDefinition: Matriz de índices de tiles
- Típico: 320×224 = 40×28 tiles = 1120 tiles
- Tamaño mapa: ~2KB de datos

**Compresión**:
- SGDK soporta LZ4H (recomendado)
- Formato: 0 (BEST) o 1 (LZ4) en directives
- Ejemplo: `MAP mymap mymap.png mymap_tileset BEST`

**Paletas**:
- 16 colores por paleta
- 4 paletas disponibles
- Guardar como .pal (JASC-PAL format) o extraer de PNG

---

## <a name="assets-de-audio"></a>

## 🔟 ASSETS DE AUDIO (SFX/MÚSICA)

### Especificación de Audio

```
┌─ RECURSO                │ TAMAÑO   │ DURACIÓN │ FORMATO │ LOOP   │ ESTADO │
├──────────────────────────┼──────────┼──────────┼─────────┼────────┼────────┤
│ MÚSICA (XGM2)            │          │          │         │        │        │
├──────────────────────────┼──────────┼──────────┼─────────┼────────┼────────┤
│ musica_fondo (Fase 3)    │ ~3-4 KB  │ 30-60s   │ VGM     │ Infinito│ ✅     │
│ musica_polo (Fase 1)     │ ~3-4 KB  │ 30-60s   │ VGM     │ Infinito│ 📋     │
│ musica_tejados (Fase 2)  │ ~3-4 KB  │ 30-60s   │ VGM     │ Infinito│ 📋     │
│ musica_celebracion (Fase 4)│ ~4-5 KB │ 30-60s   │ VGM     │ Infinito│ 📋     │
│ musica_geesebumps (Intro)│ ~3-4 KB  │ 15-20s   │ VGM     │ Sí     │ ✅     │
├──────────────────────────┼──────────┼──────────┼─────────┼────────┼────────┤
│ SFX FASE 3 (PCM)         │          │          │         │        │        │
├──────────────────────────┼──────────┼──────────┼─────────┼────────┼────────┤
│ snd_campana              │ ~25 KB   │ 0.5s     │ WAV 8k  │ No     │ ✅     │
│ snd_bomba                │ ~10 KB   │ 0.3s     │ WAV 8k  │ No     │ ✅     │
│ snd_canon                │ ~5 KB    │ 0.15s    │ WAV 8k  │ No     │ ✅     │
├──────────────────────────┼──────────┼──────────┼─────────┼────────┼────────┤
│ SFX SISTEMA (PCM)        │          │          │         │        │        │
├──────────────────────────┼──────────┼──────────┼─────────┼────────┼────────┤
│ snd_letra_ok             │ ~2 KB    │ 0.1s     │ WAV 8k  │ No     │ ✅     │
│ snd_letra_no             │ ~2.5 KB  │ 0.15s    │ WAV 8k  │ No     │ ✅     │
│ snd_victoria             │ ~107 KB  │ 2s       │ WAV 8k  │ No     │ ✅     │
│ snd_aplausos             │ ~105 KB  │ 3s       │ WAV 8k  │ Loop x2│ ✅     │
├──────────────────────────┼──────────┼──────────┼─────────┼────────┼────────┤
│ SFX FASE 1 (TODO)        │          │          │         │        │        │
├──────────────────────────┼──────────┼──────────┼─────────┼────────┼────────┤
│ snd_regalo_recogido      │ ~1.6 KB  │ 0.2s     │ WAV 8k  │ No     │ 📋     │
│ snd_disparo_red          │ ~0.8 KB  │ 0.1s     │ WAV 8k  │ No     │ 📋     │
│ snd_obstáculo_golpe      │ ~1.2 KB  │ 0.15s    │ WAV 8k  │ No     │ 📋     │
├──────────────────────────┼──────────┼──────────┼─────────┼────────┼────────┤
│ SFX FASE 2 (TODO)        │          │          │         │        │        │
├──────────────────────────┼──────────┼──────────┼─────────┼────────┼────────┤
│ snd_regalo_disparado     │ ~1.2 KB  │ 0.15s    │ WAV 8k  │ No     │ 📋     │
│ snd_entrega_exitosa      │ ~2.4 KB  │ 0.3s     │ WAV 8k  │ No     │ 📋     │
│ snd_chimenea_activa      │ ~1.6 KB  │ 0.2s     │ WAV 8k  │ No     │ 📋     │
├──────────────────────────┼──────────┼──────────┼─────────┼────────┼────────┤
│ SFX FASE 4 (TODO)        │          │          │         │        │        │
├──────────────────────────┼──────────┼──────────┼─────────┼────────┼────────┤
│ snd_confeti_choque       │ ~0.8 KB  │ 0.1s     │ WAV 8k  │ No     │ 📋     │
└──────────────────────────┴──────────┴──────────┴─────────┴────────┴────────┘
```

### Especificaciones Técnicas de Audio

**XGM2 (Música VGM)**:
```
Formato: Video Game Music (.vgm)
Compresión: VGZ (comprimido)
Canales: FM Synth (SEGA YM2612) + PSG
Tamaño típico: 3-5 KB por canción
Loop: Soportado (tag end→loop point)
Reproducción: XGM2_play(data)
Volumen: FM 0-127, PSG 0-127
```

**PCM (Efectos WAV)**:
```
Formato: WAV, sin comprimir
Frecuencia: 8kHz (bajo SGDK, máximo)
Bits: 8-bit mono
Tamaño: típico 0.5-2 segundos
Reproducción: XGM2_playPCM(data, size, channel)
Canales PCM: CH1, CH2, CH_AUTO
Máximo simultáneo: 1 PCM a la vez (XGM2)
```

### Directivas resources.res

```makefile
# MÚSICA (XGM2)
XGM2 musica_fondo                    musica.vgm
XGM2 musica_polo                     musica_polo.vgm
XGM2 musica_tejados                  musica_tejados.vgm
XGM2 musica_celebracion              musica_celebracion.vgm
XGM2 musicageesebumps                GeesebumpsGoosebumpsintro.vgm

# SFX (PCM → XGM2)
WAV snd_campana                      sndcampana.wav XGM2
WAV snd_bomba                        sndbomba.wav XGM2
WAV snd_canon                        sndcanon.wav XGM2
WAV snd_letra_ok                     sndletraok.wav XGM2
WAV snd_letra_no                     sndletrano.wav XGM2
WAV snd_victoria                     sndvictoria.wav XGM2
WAV snd_aplausos                     sndaplausos.wav XGM2
WAV snd_regalo_recogido              sndregalo_recogido.wav XGM2
WAV snd_disparo_red                  snd_disparo_red.wav XGM2
WAV snd_obstaculo_golpe              snd_obstaculo_golpe.wav XGM2
WAV snd_regalo_disparado             snd_regalo_disparado.wav XGM2
WAV snd_entrega_exitosa              snd_entrega_exitosa.wav XGM2
WAV snd_chimenea_activa              snd_chimenea_activa.wav XGM2
WAV snd_confeti_choque               snd_confeti_choque.wav XGM2
```

---

## <a name="requisitos-de-memoria"></a>

## 1️⃣1️⃣ REQUISITOS DE MEMORIA

### Presupuesto Total

```
MEMORIAM 64KB TOTAL = 65536 bytes

┌─ COMPONENTE                   │ ASIGNACIÓN │ %     │ NOTAS          │
├────────────────────────────────┼────────────┼───────┼────────────────┤
│ Stack (sistema)                │ 2 KB       │ 3.1%  │ Mínimo         │
│ Sprites activos (80)           │ 10 KB      │ 15%   │ Máximo SGDK    │
│ Maps/Fondos (2 activos)        │ 8 KB       │ 12%   │ Tiles en VRAM   │
│ Paletas (4 × 16 colores)       │ 0.25 KB    │ 0.4%  │ En VRAM         │
│ Variables globales c/minijuego │ 4 KB       │ 6%    │ Fase at a time  │
│ Buffers temporales             │ 2 KB       │ 3%    │ Para cálculos   │
│ Audio PCM (activo)             │ 0 KB       │ 0%    │ En cartucho     │
├────────────────────────────────┼────────────┼───────┼────────────────┤
│ DISPONIBLE LIBRE               │ 38.75 KB   │ 60%   │ Margen seguro   │
└────────────────────────────────┴────────────┴───────┴────────────────┘
```

### Desglose por Fase

**FASE 3 (Campanadas) - YA IMPLEMENTADA**
```
Sprites: 6 campanillas + 3 bombas + 3 balas + 1 cañón = 13 sprites
RAM: ~3 KB variables locales
VRAM: Fondos + paletas = ~16 KB
Audio: Música loop + SFX bajo demanda
Total Fase 3: ~20 KB
```

**FASE 1 (Recogida)**
```
Sprites: 6 regalos + 3 proyectiles + 1 cañón + decoración = ~12 sprites
RAM: ~3 KB variables locales
VRAM: Fondos + paletas = ~14 KB
Audio: Música loop + SFX
Estimado: ~18 KB
```

**FASE 2 (Entrega)**
```
Sprites: 15 chimeneas + 6 regalos en vuelo + 1 cañón = ~22 sprites
RAM: ~4 KB variables locales
VRAM: Fondos + paletas = ~18 KB
Audio: Música loop + SFX
Estimado: ~24 KB
```

**FASE 4 (Celebración)**
```
Sprites: 40-50 confeti + 2 personajes + 1 árbol = ~50+ sprites
RAM: ~4 KB variables locales
VRAM: Fondos + paletas = ~16 KB
Audio: Música + SFX aplausos (loop)
Estimado: ~25 KB
```

### VRAM (Video Memory) - 64KB

```
┌─ COMPONENTE               │ ASIGNACIÓN │ NOTAS                  │
├────────────────────────────┼────────────┼────────────────────────┤
│ Tileset 1 (Fondo)          │ 8 KB       │ Sprites compartidos     │
│ Tileset 2 (Paralaje)       │ 8 KB       │ Fondos + parallax       │
│ Tileset 3 (Extra)          │ 8 KB       │ Tiles decoración        │
│ Paletas (4 × 16 colores)   │ 0.25 KB    │ Simultáneas             │
│ Windows/HUD                │ 2 KB       │ Texto dinámico          │
│ Buffers libres             │ 39.75 KB   │ SGDK gestiona auto      │
└────────────────────────────┴────────────┴────────────────────────┘
```

### Optimizaciones Aplicadas

```
✅ Sprites reutilizados por fase (no simultáneos)
✅ Paletas compartidas (4 globales)
✅ Fondos loadables dinámicamente
✅ Audio: Música loop única (economiza ROM no RAM)
✅ PCM comprimido en cartucho
✅ Sin buffering innecesario
✅ Arrays estáticos preasignados (no malloc)
✅ Variables u8/s8 (no int)
```

---

## <a name="máquina-de-estados"></a>

## 1️⃣2️⃣ MÁQUINA DE ESTADOS

### Estados Globales

```
STATE MACHINE (main.c):

   START
     │
     ├──→ INTRO_GEESEBUMPS ═══════════════════════════════════════════════╗
     │      ├─ Mostrar logo splash screen                                │
     │      ├─ Reproducir música intro                                   │
     │      ├─ Duración: 3-5 segundos                                    │
     │      └─ Transición: AUTO tras tiempo                             │
     │                                                                    │
     ├──→ PHASE_PICKUP (Fase 1) ════════════════════════════════════════╗
     │      ├─ init: minigamePickup_init()                              │
     │      ├─ loop: while !isComplete()                                │
     │      │        ├─ minigamePickup_update()                         │
     │      │        ├─ minigamePickup_render()                         │
     │      │        └─ SPR_update(); SYS_doVBlankProcess()            │
     │      ├─ Objetivo: 20 regalos                                     │
     │      └─ Siguiente: PHASE_DELIVERY                                │
     │                                                                    │
     ├──→ PHASE_DELIVERY (Fase 2) ══════════════════════════════════════╗
     │      ├─ init: minigameDelivery_init()                            │
     │      ├─ loop: while !isComplete()                                │
     │      │        ├─ minigameDelivery_update()                       │
     │      │        ├─ minigameDelivery_render()                       │
     │      │        └─ SPR_update(); SYS_doVBlankProcess()            │
     │      ├─ Objetivo: 10 entregas                                    │
     │      └─ Siguiente: PHASE_BELLS                                   │
     │                                                                    │
     ├──→ PHASE_BELLS (Fase 3) ═════════════════════════════════════════╗
     │      ├─ init: minigameBells_init()                               │
     │      ├─ loop: while !isComplete()                                │
     │      │        ├─ minigameBells_update()                          │
     │      │        ├─ minigameBells_render()                          │
     │      │        └─ SPR_update(); SYS_doVBlankProcess()            │
     │      ├─ Objetivo: 12 campanillas                                 │
     │      └─ Siguiente: PHASE_CELEBRATION                             │
     │                                                                    │
     ├──→ PHASE_CELEBRATION (Fase 4) ════════════════════════════════════╗
     │      ├─ init: minigameCelebration_init()                         │
     │      ├─ loop: while !isComplete() OR timeout                     │
     │      │        ├─ minigameCelebration_update()                    │
     │      │        ├─ minigameCelebration_render()                    │
     │      │        └─ SPR_update(); SYS_doVBlankProcess()            │
     │      ├─ Duración: 15-30 segundos                                 │
     │      └─ Siguiente: PHASE_GAMEOVER                                │
     │                                                                    │
     └──→ PHASE_GAMEOVER
            ├─ Mostrar "¡FELIZ 2026!"
            ├─ Esperar botón
            └─ END / RESTART
```

### Estados Internos de Fase 3

```
MINIGAME INTERNAL STATE (minigame_bells.c):

   INIT (Fase 3 comienza)
     │
     ├──→ PLAYING (Jugando)
     │      ├─ Actualizar campanillas
     │      ├─ Actualizar bombas
     │      ├─ Procesar entrada
     │      ├─ Detectar colisiones
     │      ├─ Renderizar sprites
     │      └─ Contar campanillas completadas
     │
     ├──→ [Loop continuo hasta isComplete() = true]
     │      └─ bellsCompleted >= NUM_FIXED_BELLS (12)
     │
     └──→ VICTORY (Victoria detectada)
            ├─ Animar parpadeo final
            ├─ Mostrar mensaje
            ├─ Fade a negro
            └─ Retornar a main.c
```

### Transiciones y Condiciones

```
┌─ DESDE              │ CONDICIÓN                      │ A                │
├─────────────────────┼────────────────────────────────┼──────────────────┤
│ INTRO_GEESEBUMPS    │ 3 segundos o botón            │ PHASE_PICKUP     │
│ PHASE_PICKUP        │ regalosRecolectados >= 20      │ PHASE_DELIVERY   │
│ PHASE_DELIVERY      │ entregasCompletadas >= 10      │ PHASE_BELLS      │
│ PHASE_BELLS         │ bellsCompleted >= 12           │ PHASE_CELEBRATION│
│ PHASE_CELEBRATION   │ Timeout 30s o botón           │ PHASE_GAMEOVER   │
│ PHASE_GAMEOVER      │ Botón START/confirmación       │ END/RESTART      │
└─────────────────────┴────────────────────────────────┴──────────────────┘
```

### Máquina de Estados de Entrada

```
INPUT HANDLER (gameCore_readInput):

   JOY_readJoypad(JOY1) → u16 input
        │
        ├──→ LEFT   (input & BUTTON_LEFT)
        ├──→ RIGHT  (input & BUTTON_RIGHT)
        ├──→ A      (input & BUTTON_A) - Disparo
        ├──→ B      (input & BUTTON_B) - Contacto/Acción
        ├──→ START  (input & BUTTON_START) - Pausa/Menú
        └──→ SELECT (input & BUTTON_SELECT) - Opcional

   Procesa cada frame (60 FPS = 16.67 ms por frame)
   Actualiza: Movimiento, disparo, colisiones
```

---

## 📝 RESUMEN EJECUTIVO

### Proyecto FELIZ 2026 - Estadísticas Finales

```
DISTRIBUCIÓN IMPLEMENTACIÓN:

Fase 3 (Campanadas):     ✅ 100% COMPLETADA   (minigame_bells.c)
Fases 1, 2, 4:           📋 Placeholders      (minigame_pickup/delivery/celebration.c)

Game Core:               ✅ 75% Reutilizable  (game_core.c)
Main Orchestrator:       ✅ Listo             (main.c)
Intro Logo:              ✅ Listo             (geesebumps.c)

Sprites Fase 3:          ✅ Completos          (~5 sprites)
Fondos Fase 3:           ✅ Completos          (2 layers)
Audio Fase 3:            ✅ Completo           (Música + 3 SFX)

Sprites Fases 1-4:       📋 Diseño pendiente   (~20+ sprites)
Fondos Fases 1-4:        📋 Diseño pendiente   (8 backgrounds)
Audio Fases 1-4:         📋 Composición pte    (4 canciones + 11 SFX)

MEMORIA DISPONIBLE:      ✅ Dentro de límites  (38KB libres)
FPS:                     ✅ Estable           (60 FPS constante)
```

---

**Documento Completo - Especificación Técnica FELIZ 2026**
**Fecha**: Diciembre 2025
**Estado**: Listo para implementación Fases 1, 2, 4
