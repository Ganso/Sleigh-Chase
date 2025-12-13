# 🏠 FASE 2 - ENTREGA (TEJADOS) - ESPECIFICACIÓN ULTRAPRECISA

**Especificación Técnica Detallada para Implementación**

---

## 📋 TABLA DE CONTENIDOS

1. [Descripción General y Narrativa](#descripción-general)
2. [Mecánicas de Juego Detalladas](#mecánicas)
3. [Especificaciones de Sprites](#sprites)
4. [Especificaciones de Fondos](#fondos)
5. [Especificaciones de Audio](#audio)
6. [Lógica de Colisiones](#colisiones)
7. [Máquina de Estados Interna](#estado-interno)
8. [Estructura de Datos C](#estructura-datos)
9. [Presupuesto de Memoria](#memoria)
10. [Checklist de Implementación](#checklist)

---

## <a name="descripción-general"></a>

## 1. DESCRIPCIÓN GENERAL Y NARRATIVA

### Contexto Narrativo
```
ACTO 1: Recogida en Polo Norte (COMPLETADA)
   ↓
ACTO 2: ENTREGA EN TEJADOS (ESTA FASE)
   ↓
ACTO 3: Campanadas en Iglesia (IMPLEMENTADA)
   ↓
ACTO 4: Celebración en Fiesta

NARRATIVA FASE 2:
─────────────────────────────────────────────────────────────
Has recolectado exitosamente 20 regalos en el Polo Norte.
Ahora debes entregarlos en las casas de la ciudad nocturna,
lanzando los regalos por las chimeneas desde los tejados.

La noche es fría, el viento sopla, y tienes tiempo limitado
para entregar 10 de tus 20 regalos en chimeneas objetivo.

¡Sé rápido y preciso!
─────────────────────────────────────────────────────────────
```

### Estado de implementación (baseline en código)

- Fondo `image_fondo_tejados` cargado en `BG_B` con cámara sobre un ancho de 512 px y overlay de nieve reutilizando `snow_effect`.
- Santa se mueve libremente por toda la pantalla con inercia compartida y puede soltar regalos con **A** al situarse sobre una chimenea.
- Ocho chimeneas de 48×48 px colocadas en coordenadas fijas (mitad izquierda/derecha) que entran en cooldown tras recibir un regalo.
- Contador de regalos inicializado a **10** unidades; cada entrega resta uno hasta completar la fase.
- Enemigos placeholder basados en el sprite de duende: al colisionar con Santa se pausa la acción y se reinicia la posición tras un breve parpadeo.

### Especificaciones de Fase

| Aspecto | Valor | Detalles |
|---------|-------|----------|
| **Nombre** | Entrega | Tejados nocturnos |
| **Ubicación** | Ciudad (tejados) | Noche, vista lateral |
| **Duración** | 60-90 seg | Variable según dificultad |
| **Objetivo** | 10 entregas | De 20 regalos totales |
| **Dificultad** | Media | Más desafiante que Fase 1 |
| **Mechanic** | Precision | Timing + Trayectoria |
| **Chimeneas** | 15 totales | 5-7 activas simultáneamente |
| **Pantalla** | 320×224 | Estándar Mega Drive |
| **Scroll** | Ninguno | Fondo estático |
| **Parallax** | Sí | Nubes lento |

---

## <a name="mecánicas"></a>

## 2. MECÁNICAS DE JUEGO DETALLADAS

### 2.1 Sistema de Control

```
ENTRADA DE JUGADOR:

Izquierda (LEFT):
├─ Acción: Mover cañón a la izquierda
├─ Velocidad: -6 píxeles/frame (aceleración suave)
├─ Límite: x ≥ 32 píxeles (margen de borde)
└─ Friction: 0.85 (desaceleración cuando sueltas)

Derecha (RIGHT):
├─ Acción: Mover cañón a la derecha
├─ Velocidad: +6 píxeles/frame (aceleración suave)
├─ Límite: x ≤ 256 píxeles (margen de borde)
└─ Friction: 0.85 (desaceleración cuando sueltas)

Botón A:
├─ Acción: DISPARAR regalo
├─ Cooldown: 25 frames (416 ms a 60 FPS)
├─ Velocidad regalo: vY = -4 píxeles/frame (inicial)
├─ Ángulo: 90° (recto hacia arriba)
├─ Máx simultáneos: 6 regalos en vuelo
└─ SFX: snd_regalo_disparado (prioridad alta)

Botón B:
├─ Acción: Contacto directo (opcional, no usar)
└─ Estado: Deshabilitado en Fase 2

START:
├─ Acción: Pausa
└─ Implementación: Futura (opcional)

SELECT:
└─ Acción: Ninguna

────────────────────────────────────────────
FÍSICA DE MOVIMIENTO:

Cañón Tejado:
├─ Posición: x = 160 (centro por defecto)
├─ Altura: y = 180 (fijo, no se mueve verticalmente)
├─ Velocidad máx: vX = ±6 píxeles/frame
├─ Aceleración: 1 píxel/frame² cuando presionas
├─ Fricción: multiplica por 0.85 cuando sueltas
└─ Sprite size: 64×96 píxeles

Regalos Lanzados:
├─ Velocidad inicial: vX = 0, vY = -4 píxeles/frame
├─ Gravedad: aY = +0.3 píxeles/frame² (hacia abajo)
├─ Viento: afecta vX con variación -0.5 a +0.5 píxeles/frame
├─ Velocidad máx caída: vY = +6 píxeles/frame
├─ Rotación: Gira lentamente (animación visual)
├─ Sprite size: 24×24 píxeles
└─ Vida: Desaparece si sale de pantalla

────────────────────────────────────────────
DETECCIÓN DE ENTREGA:

Condiciones para +1 ENTREGA:
├─ Regalo.y ≥ Chimenea.y (regalo alcanza altura de chimenea)
├─ |Regalo.x - Chimenea.x| ≤ 20 píxeles (dentro de ancho)
├─ Regalo.vY ≥ 0 (regalo cayendo, no subiendo)
└─ Chimenea.activa == true

Efectos inmediatos:
├─ SFX: snd_entrega_exitosa (prioridad alta)
├─ VFX: Parpadeo de chimenea (30 frames)
├─ HUD: Incrementar contador "Entregas: X/10"
├─ Regalo: Desaparecer de pantalla
└─ Chimenea: Marcar como iluminada/completada
```

### 2.2 Sistema de Chimeneas

```
CHIMENEAS - DISTRIBUCIÓN Y COMPORTAMIENTO:

Total de chimeneas: 15 distribuidas por pantalla
Simultáneas activas: 5-7 (varía por tiempo y dificultad)

POSICIONES FIJAS (x,y):
┌─ Chimenea  │ Pos X │ Pos Y │ Fase Activación │ Estado Inicial │
├─────────────┼───────┼───────┼─────────────────┼────────────────┤
│ Chimenea 0  │ 30    │ 100   │ 0-30s           │ Activa         │
│ Chimenea 1  │ 70    │ 110   │ 0-30s           │ Activa         │
│ Chimenea 2  │ 110   │ 95    │ 0-30s           │ Activa         │
│ Chimenea 3  │ 150   │ 105   │ 0-30s           │ Activa         │
│ Chimenea 4  │ 190   │ 90    │ 0-30s           │ Activa         │
│ Chimenea 5  │ 230   │ 100   │ 20-50s          │ Inactiva 20s   │
│ Chimenea 6  │ 270   │ 95    │ 20-50s          │ Inactiva 20s   │
│ Chimenea 7  │ 310   │ 105   │ 20-50s          │ Inactiva 20s   │
│ Chimenea 8  │ 45    │ 75    │ 40-70s          │ Inactiva 40s   │
│ Chimenea 9  │ 95    │ 70    │ 40-70s          │ Inactiva 40s   │
│ Chimenea 10 │ 155   │ 80    │ 40-70s          │ Inactiva 40s   │
│ Chimenea 11 │ 215   │ 65    │ 40-70s          │ Inactiva 40s   │
│ Chimenea 12 │ 275   │ 75    │ 40-70s          │ Inactiva 40s   │
│ Chimenea 13 │ 320   │ 70    │ Nunca (Extra)   │ Siempre activa │
│ Chimenea 14 │ 160   │ 85    │ Siempre         │ Siempre activa │
└─────────────┴───────┴───────┴─────────────────┴────────────────┘

TAMAÑO DE HITBOX:
├─ Ancho efectivo: 20 píxeles (centro chimenea ±10)
├─ Alto efectivo: 40 píxeles (cuello chimenea)
└─ Visual: 20×40 píxeles en pantalla

ESTADOS DE CHIMENEA:
├─ 0: INACTIVA (no cuenta)
├─ 1: ACTIVA (objetivo actual, gris oscuro)
├─ 2: COMPLETADA (iluminada, brilla)
└─ 3: PARPADEANDO (animación tras completar)

ANIMACIONES:
├─ Inactiva: Sprite estático gris
├─ Activa: Sprite normal rojo/ladrillo
├─ Completada: Parpadea 30 frames, emite luz
└─ Sprite: 2 frames (apagada, encendida)
```

### 2.3 Dificultad Progresiva

```
TIMELINE DE DIFICULTAD (90 segundos total):

BLOQUE 1: 0-30 SEGUNDOS (FÁCIL - TUTORIAL)
├─ Chimeneas activas: 5 (todas en fila)
├─ Generación regalos: 2 cada 5 segundos
├─ Viento: -0.2 a +0.2 píxeles/frame (suave)
├─ Velocidad caída: Normal
├─ Objetivo acumulado: 3 entregas
└─ SFX: Música suave, SFX claros

BLOQUE 2: 30-60 SEGUNDOS (MEDIA - RETOS)
├─ Chimeneas activas: 7 (distribuidas)
├─ Chimeneas fase 1: Apagadas/remodeladas
├─ Generación regalos: 3 cada 4 segundos
├─ Viento: -0.5 a +0.5 píxeles/frame (moderado)
├─ Velocidad caída: Aumenta 5%
├─ Objetivo acumulado: 6-7 entregas
└─ SFX: Música acelera, más SFX activos

BLOQUE 3: 60-90 SEGUNDOS (DIFÍCIL - SPRINT FINAL)
├─ Chimeneas activas: 10+ (máximo caos)
├─ Chimeneas previas: Alternadamente se encienden
├─ Generación regalos: 4 cada 3 segundos
├─ Viento: -1.0 a +1.0 píxeles/frame (fuerte)
├─ Velocidad caída: Aumenta 10% más
├─ Objetivo acumulado: 10 entregas (VICTORIA)
└─ SFX: Música frenetica, caos

VARIABLES DINÁMICAS:

Contador de Tiempo:
├─ frameCounter incrementa cada frame
├─ tiempo_segundos = frameCounter / 60
├─ Chequear cada 1800 frames (30 segundos)

Velocidad viento:
├─ wind_factor = (sin(frameCounter * 0.02) * 0.5) + offset_bloque
├─ Rango aumenta por bloque
└─ Aplicar a vX regalo: regalo.vX += wind_factor

Dificultad multiplicador:
├─ difficulty_mult = 1.0 + (tiempo_segundos / 100)
├─ Afecta velocidad caída
└─ Max 1.9x en segundo 90
```

### 2.4 Sistema de Score y Progreso

```
HUD - INTERFAZ DE USUARIO:

Posición: Esquina superior derecha (inicio x=250, y=10)
Tamaño: 60×60 píxeles
Contenido:
├─ Título: "Entregas:"
├─ Número: X/10 (contador principal)
├─ Barra: Visual bar 40×8 píxeles
└─ Color: Verde al 100%, rojo al 0%

CONTADOR VISUAL:

    ┌────────────────────────────┐
    │ ENTREGAS: 7/10             │
    │ ▓▓▓▓▓▓▓░░░                 │
    └────────────────────────────┘

Cambios por estado:
├─ +1 Entrega exitosa: Incremento número + SFX
├─ 10/10: Flash de victoria, música cambia
├─ Visual: Barra se llena progresivamente
└─ Color transición: Verde (0-5), Amarillo (5-8), Rojo vivo (8-10)

CONDICIONES DE VICTORIA:

Victoria inmediata:
├─ entregasCompletadas >= 10
├─ Reproducir snd_victoria
├─ Fade a negro 60 frames
├─ Mostrar "¡VICTORIA!" 120 frames
├─ Transición automática a Fase 3
└─ markedForCompletion = true

Si NO hay victoria en 90 segundos:
├─ Timeout: NO ocurre (sin límite de tiempo real)
├─ Pero la dificultad máxima se alcanza
├─ Jugador debe completar manualmente
└─ Sin penalización por tiempo
```

---

## <a name="sprites"></a>

## 3. ESPECIFICACIONES DE SPRITES

### 3.1 sprite_regalo_entrega

```
NAME:        sprite_regalo_entrega
SIZE:        24×24 píxeles
FRAMES:      1 (estático)
PALETTE:     PAL_PLAYER (1)
SOURCE FILE: Regalos_Pequeño.png

DESCRIPCIÓN:
├─ Regalo pequeño envuelto
├─ Colores: Rojo brillante (#FF0000), cinta dorada (#FFD700)
├─ Forma: Cúbica con lazo encima
└─ Transparencia: Magenta (#FF00FF) = fondo

ANIMACIÓN:
├─ No tiene (1 frame único)
├─ Rotación lenta en juego (software, no sprite)
└─ Rotación: 6° por frame

RENDERIZADO:
├─ Depth: DEPTH_ACTORS
├─ Priority: 2
├─ Visibility: Visible
└─ Blending: Opaco

USO EN CÓDIGO:
├─ Crear: sprite = SPR_addSprite(&sprite_regalo_entrega, x, y, TILE_ATTR(...)
├─ Mover: SPR_setPosition(sprite, x, y)
├─ Destruir: SPR_releaseSprite(sprite)
└─ Cantidad: Máximo 6 simultáneos
```

### 3.2 sprite_chimenea

```
NAME:        sprite_chimenea
SIZE:        20×80 píxeles (frame 0), 20×80 (frame 1)
FRAMES:      2 (apagada, encendida)
PALETTE:     PAL_PLAYER (1)
SOURCE FILE: Chimenea.png (20×160 total)

DESCRIPCIÓN VISUAL:

Frame 0 (APAGADA):
├─ Ladrillo gris oscuro (#404040)
├─ Líneas mortar blanco (#CCCCCC)
└─ Aspecto: Frío, oscuro

Frame 1 (ENCENDIDA):
├─ Ladrillo rojo oscuro (#990000)
├─ Humo naranja (#FF6600) saliendo
├─ Brillo internal (#FFAA00)
└─ Aspecto: Cálido, activo

ANIMACIÓN:
├─ Frame 0 por defecto (inactiva/completada)
├─ Frame 1 cuando recibe regalo exitoso
├─ Parpadea Frame 0↔1 cada 15 frames tras victoria
└─ Vuelve a Frame 1 si recibe otro

RENDERIZADO:
├─ Depth: DEPTH_BACKGROUND
├─ Priority: 1
├─ Visibility: Visible
└─ No scroll

USO EN CÓDIGO:
├─ Crear: chimenea.sprite = SPR_addSprite(&sprite_chimenea, chimenea.x, chimenea.y, ...)
├─ Estado: sprite_frame = chimenea.active ? 1 : 0
├─ Animar: SPR_setAnimAndFrame(chimenea.sprite, 0, sprite_frame)
└─ Cantidad: 15 máximo (generalmente 10-12 visibles)

HITBOX:
├─ Ancho real: 20 píxeles
├─ Alto real: 80 píxeles
├─ Punto de referencia: Esquina superior izquierda
└─ Offset center: +10 píxeles horizontalmente
```

### 3.3 sprite_canon_tejado

```
NAME:        sprite_canon_tejado
SIZE:        64×96 píxeles (frame 0 reposo, frame 1 disparando)
FRAMES:      2 (reposo, disparo)
PALETTE:     PAL_PLAYER (1)
SOURCE FILE: CanonTejado.png (128×96 total)

DESCRIPCIÓN VISUAL:

Frame 0 (REPOSO):
├─ Cañón de madera plegado
├─ Trineo rojo con detalles
├─ Posición: Horizontal/diagonal
└─ Colores: Madera (#8B4513), Rojo (#FF0000), Metal gris (#808080)

Frame 1 (DISPARANDO):
├─ Cañón en retroceso
├─ Chispa/destello naranja
├─ Posición: Ligeramente hacia atrás
└─ Animación rápida: 5 frames

ANIMACIÓN:
├─ Por defecto: Frame 0
├─ Al presionar A: SPR_setAnimAndFrame(cannon, 0, 1)
├─ Duración: 5 frames
├─ Vuelve a Frame 0 automáticamente
└─ Cooldown: 25 frames antes de poder disparar de nuevo

RENDERIZADO:
├─ Depth: DEPTH_ACTORS
├─ Priority: 3 (sobre fondos)
├─ Visibility: Siempre visible
└─ Blending: Opaco

USO EN CÓDIGO:
├─ Crear: cannonTejado = SPR_addSprite(&sprite_canon_tejado, 160, 180, ...)
├─ Mover: SPR_setPosition(cannonTejado, newX, 180)
├─ Disparar: SPR_setAnimAndFrame(cannonTejado, 0, 1); frameDisparoActual = 0;
├─ Update: if(frameDisparoActual++ >= 5) { SPR_setAnimAndFrame(cannonTejado, 0, 0); }
└─ Cantidad: 1 único
```

### 3.4 sprite_nube

```
NAME:        sprite_nube
SIZE:        64×32 píxeles
FRAMES:      1 (estático)
PALETTE:     PAL_COMMON (0)
SOURCE FILE: Nube.png

DESCRIPCIÓN:
├─ Nube blanca algodonosa
├─ Colores: Blanco (#FFFFFF), Gris suave (#DDDDDD)
├─ Forma: Cúmulo esponjoso
└─ Transparencia: Magenta (#FF00FF)

ANIMACIÓN:
├─ No tiene (1 frame único)
├─ Movimiento: Paralaje lento (software)
└─ Velocidad parallax: -0.5 píxeles/frame

RENDERIZADO:
├─ Depth: DEPTH_BACKGROUND
├─ Priority: 0 (detrás de todo)
├─ Visibility: Visible
└─ Parallax BG offset: sí

USO EN CÓDIGO:
├─ Renderizar: En fondo paralaje (no sprite individual)
├─ Técnica: Tilemap con scroll lento
└─ Cantidad: 10+ nubes en patrón

POSICIONAMIENTO:
├─ x inicial: 0 a 640 (ancho pantalla extendido)
├─ y: Fijo (100 píxeles aprox)
└─ Reinicio: Cuando x < -64
```

### 3.5 sprite_regalo_volador (interno, no sprite visual)

```
DESCRIPCIÓN:
├─ Regalo en vuelo (logically managed, not visual sprite)
├─ Usa sprite_regalo_entrega para renderizar
├─ Estructura de datos separada para física
└─ Máximo 6 simultáneos

PROPIEDADES:
├─ x, y: Posición actual
├─ vx, vy: Velocidad
├─ rotation: Rotación en grados (0-360)
├─ active: Booleano
├─ createdFrames: Tiempo de vida

UPDATE CADA FRAME:
├─ rotation += 6
├─ if(rotation >= 360) rotation = 0
├─ vx += wind_factor
├─ vy += gravity (0.3 píxeles/frame²)
├─ x += vx
├─ y += vy
├─ Límites pantalla: Si y > 240 → inactive
```

---

## <a name="fondos"></a>

## 4. ESPECIFICACIONES DE FONDOS

### 4.1 image_fondo_tejados (Capa BG_B)

```
NAME:          image_fondo_tejados
TYPE:          TileSet + MapDefinition
SIZE:          320×224 píxeles de pantalla
TILESET SIZE:  512×512 píxeles (mínimo)
COMPRESSION:   BEST (máxima compresión)
PALETTE:       PAL_COMMON (0)
SOURCE FILES:  FondoTejados.png (512×256), FondoTejados.tileset, FondoTejados.map

CONTENIDO VISUAL:
├─ Tejados variados (rojo, marrón, gris)
├─ Chimeneas destacadas en diferentes posiciones
├─ Luces de casa parpadeantes (oranges, amarillos)
├─ Luna llena en esquina superior
├─ Estrellas dispersas
├─ Línea del horizonte con árboles distantes
└─ Atmósfera nocturna (colores azules oscuros)

PALETA RECOMENDADA:
┌─ Índice │ Color            │ Uso                │
├─────────┼──────────────────┼────────────────────┤
│ 0       │ Negro (#000000)  │ Fondo/Transparente │
│ 1-4     │ Azules oscuros   │ Cielo nocturno     │
│ 5-8     │ Grises           │ Tejados neutrales  │
│ 9-12    │ Rojos/Marrones   │ Tejados principales│
│ 13-15   │ Amarillos/Blancos│ Luces casa         │
└─────────┴──────────────────┴────────────────────┘

TILESET ORGANIZATION:
├─ Tile 0: Negro (transparente)
├─ Tiles 1-16: Variaciones cielo/atmósfera
├─ Tiles 17-48: Tejados comunes
├─ Tiles 49-64: Chimeneas/detalles
├─ Tiles 65-80: Luces/efectos
└─ Tiles 81+: Decoración adicional

RENDERING:
├─ Parallax: NO
├─ Scroll: NO
├─ Priority: 0 (detrás)
├─ Map size: 40×28 tiles (320÷8 × 224÷8)
└─ Mapa data: ~2.2KB sin comprimir
```

### 4.2 image_nubes_paralaje (Capa BG_A overlay)

```
NAME:          image_nubes_paralaje
TYPE:          TileSet + MapDefinition (parallax)
SIZE:          320×224 píxeles de pantalla
TILESET SIZE:  256×256 píxeles
COMPRESSION:   BEST
PALETTE:       PAL_COMMON (0) - compartida con fondo
SOURCE FILES:  Nubes.png, Nubes.tileset, Nubes.map

CONTENIDO VISUAL:
├─ Nubes blancas
├─ Niebla sutil
├─ Efectos atmosféricos
└─ Semi-transparencia (usando paleta suave)

PARALLAX IMPLEMENTATION:
├─ Velocidad: -0.5 píxeles/frame (muy lento)
├─ Dirección: Horizontal (derecha a izquierda)
├─ Distancia parallax: Lejana (nubes de fondo)
├─ Loop: Automático (cuando completa rotación)
├─ Offset inicial: 0
└─ Update: offset = (offset + 0.5) % 512

RENDERING:
├─ Layer: BG_A (sobre BG_B)
├─ Parallax: Sí
├─ Priority: Medio
├─ Blending: Opaco
└─ Update: BG_setHorizontalScroll(BG_A, (u16)offset)
```

---

## <a name="audio"></a>

## 5. ESPECIFICACIONES DE AUDIO

### 5.1 Música - reutilización temporal

```
TRACK:          musica_fondo (campanas Fase 3)
FORMATO:        XGM2 (VGM - Video Game Music)
DURACIÓN:       40-60 segundos en loop infinito
VOLUMEN:        FM=70/127, PSG=100/127

NOTA:           La fase de tejados reutiliza provisionalmente la misma
                pista que la fase de campanadas para acelerar pruebas y
                evitar placeholders WAV. Se mantiene la reproducción en
                loop continuo.

REPRODUCCIÓN EN CÓDIGO:
├─ Iniciar: XGM2_play(musica_fondo)
├─ Con loop: Automático (flag en VGM)
├─ Parar: XGM2_stop()
└─ Fade: XGM2_fadeOut(tiempo_frames)
```

### 5.2 SFX - snd_regalo_disparado

```
NAME:           snd_regalo_disparado
FORMAT:         WAV (sin comprimir)
ENCODING:       8-bit PCM mono
SAMPLE RATE:    8000 Hz
DURATION:       ~0.15 segundos (120 samples)
FILE SIZE:      ~1.2 KB
FILE:           snd_regalo_disparado.wav

DESCRIPCIÓN SONORA:
├─ Tipo: Sonido de lanzamiento suave
├─ Pitch: Bajo (100-200 Hz)
├─ Envolvente: Ataque rápido, decay medio
└─ Carácter: "Whomp" o "Thud" amortiguado

REPRODUCCIÓN EN CÓDIGO:
├─ Trigger: Al presionar A (cada 25 frames mínimo)
├─ Función: XGM2_playPCM(snd_regalo_disparado, sndregaloDisparado_size, SOUND_PCM_CH_AUTO)
├─ Prioridad: Alta (importante para feedback)
├─ Volumen: 100%
└─ Cantidad simultánea: Máximo 1 (otros descartados)

LIMITACIONES:
├─ XGM2 permite 1 PCM simultáneo
├─ Si intenta reproducir otro: Se reemplaza el anterior
└─ Solución: Controlar con frameLastSFX
```

### 5.3 SFX - snd_entrega_exitosa

```
NAME:           snd_entrega_exitosa
FORMAT:         WAV (sin comprimir)
ENCODING:       8-bit PCM mono
SAMPLE RATE:    8000 Hz
DURATION:       ~0.3 segundos
FILE SIZE:      ~2.4 KB
FILE:           snd_entrega_exitosa.wav

DESCRIPCIÓN SONORA:
├─ Tipo: Fanfarra de éxito corta
├─ Pitch: Dos notas ascendentes (Do-Mi)
├─ Envolvente: Ataque suave, sustain medio, decay rápido
└─ Carácter: "Ding-dong" campana feliz

MELODÍA RECOMENDADA:
├─ Nota 1: Do (262 Hz) - 150 ms
├─ Nota 2: Mi (330 Hz) - 150 ms
└─ Total: 300 ms

REPRODUCCIÓN EN CÓDIGO:
├─ Trigger: Cuando regalo toca chimenea activa
├─ Función: XGM2_playPCM(snd_entrega_exitosa, sndentregaExitosa_size, SOUND_PCM_CH_AUTO)
├─ Prioridad: Muy alta (más importante que disparo)
├─ Volumen: 120% (boost)
└─ Cantidad: 1 simultáneo

FRECUENCIA REPRODUCCIÓN:
├─ Máximo: 1 por frame (si múltiples entregas = solo 1 SFX)
├─ Acumulado durante fase: 10 máximo (victoria)
└─ Duración total: 3 segundos si 10 consecutivas
```

### 5.4 SFX - snd_chimenea_activa

```
NAME:           snd_chimenea_activa
FORMAT:         WAV (sin comprimir)
ENCODING:       8-bit PCM mono
SAMPLE RATE:    8000 Hz
DURATION:       ~0.2 segundos
FILE SIZE:      ~1.6 KB
FILE:           snd_chimenea_activa.wav

DESCRIPCIÓN SONORA:
├─ Tipo: Efecto de encendido/fuego
├─ Pitch: Pop/crackle (ruido de fuego)
├─ Envolvente: Ataque muy rápido, decay rápido
└─ Carácter: "Pop" o "Whoosh" de fuego

REPRODUCCIÓN EN CÓDIGO:
├─ Trigger: Cuando se activa chimenea nueva
├─ Frecuencia: Máximo 1 por 30 frames (no abrumar)
├─ Función: XGM2_playPCM(snd_chimenea_activa, sndchimenea_size, SOUND_PCM_CH_AUTO)
├─ Prioridad: Media (menos importante que entrega)
├─ Volumen: 80%
└─ Cantidad: 1 simultáneo (comparte canal con otros)

IMPLEMENTACIÓN:
├─ Solo reproducir si chimenea pasa a activa (state change)
├─ No reproducir si ya estaba activa
├─ Cooldown: 30 frames mínimo entre reproducciones
└─ Máximo 7 SFX durante fase (máximo chimeneas activas)
```

---

## <a name="colisiones"></a>

## 6. LÓGICA DE COLISIONES

### 6.1 Sistema AABB (Axis-Aligned Bounding Box)

```
FUNCIÓN BASE:
────────────────────────────────────────────────────────────
u8 gameCore_checkCollisionAABB(s16 x1, s16 y1, u16 w1, u16 h1,
                               s16 x2, s16 y2, u16 w2, u16 h2)
────────────────────────────────────────────────────────────

Condición: Colisión si:
├─ x1 < x2+w2  AND  x1+w1 > x2  AND
├─ y1 < y2+h2  AND  y1+h1 > y2
└─ Resultado: 1 (colisión) o 0 (sin colisión)

APLICACIÓN EN FASE 2:

Colisión Regalo-Chimenea:
├─ Box regalo: (regalo.x-12, regalo.y-12, 24, 24)
├─ Box chimenea: (chimenea.x-10, chimenea.y, 20, 80)
├─ Si colisión: +1 entrega
└─ Velocidad Y debe ser >= 0 (cayendo, no subiendo)

Colisión Regalo-Pantalla:
├─ Box pantalla: (0, 0, 320, 224)
├─ Si regalo.y > 240: marca como inactivo
├─ Si regalo.x < 0 o > 320: marca como inactivo
└─ Efecto: Regalo desaparece sin puntuación
```

### 6.2 Detección de Entrega Detallada

```
PSEUDOCÓDIGO DETECCIÓN:
────────────────────────────────────────────────────────────

for each regalo in regalos_activos:
    if regalo.active == false:
        continue
    
    for each chimenea in chimeneas:
        if chimenea.active == false:
            continue
        
        // Verificar posición Y (regalo alcanza altura)
        if regalo.y < chimenea.y - 40:
            continue  // Regalo aún arriba
        
        // Verificar rango horizontal
        distancia_x = abs(regalo.x - chimenea.x)
        if distancia_x > 20:
            continue  // Fuera de rango horizontal
        
        // Verificar dirección de movimiento (cayendo)
        if regalo.vy < 0:
            continue  // Regalo subiendo, no cuenta
        
        // COLISIÓN DETECTADA
        entregar_regalo_en_chimenea(regalo, chimenea)
        break  // Exit chimenea loop

────────────────────────────────────────────────────────────

FUNCIÓN ENTREGA:
────────────────────────────────────────────────────────────

void entregar_regalo_en_chimenea(Regalo* regalo, Chimenea* chimenea) {
    // Incrementar contador
    entregasCompletadas++
    
    // Reproducir SFX
    XGM2_playPCM(snd_entrega_exitosa, size, CH_AUTO)
    
    // Marcar chimenea como completada
    chimenea.estado = STATE_COMPLETADA
    chimenea.frameParpadeo = 30  // Parpadea 30 frames
    
    // Marcar regalo como usado
    regalo.active = false
    SPR_releaseSprite(regalo.sprite)
    
    // Actualizar HUD
    actualizarHUD_entregas(entregasCompletadas)
    
    // Verificar victoria
    if (entregasCompletadas >= 10) {
        minigame_celebration_ready = true
        markedForCompletion = true
    }
}

────────────────────────────────────────────────────────────
```

### 6.3 Detección de Límites Pantalla

```
LÍMITES CAÑÓN:

Movimiento horizontal:
├─ Mínimo x: 32 píxeles (margen izquierdo)
├─ Máximo x: 256 píxeles (320 - 64 ancho cañón)
├─ Si x < 32: x = 32
├─ Si x > 256: x = 256
└─ Sprite y: siempre 180

LÍMITES REGALO:

Fuera de pantalla:
├─ Si regalo.y > 240: marcar inactivo
├─ Si regalo.x < -50: marcar inactivo
├─ Si regalo.x > 370: marcar inactivo
└─ Efecto: Desaparece sin entrega (fallo silencioso)

LÍMITES CHIMENEA:

Siempre en pantalla:
├─ x: 30 a 310 píxeles
├─ y: 65 a 110 píxeles
└─ No se mueven, posiciones fijas
```

---

## <a name="estado-interno"></a>

## 7. MÁQUINA DE ESTADOS INTERNA

### 7.1 Estados Globales de Fase

```
ESTADO: minigameDelivery_state

STATE_INIT (0):
├─ Acción: Inicializar todos los recursos
├─ Duración: 1 frame
├─ Siguiente: STATE_PLAYING

STATE_PLAYING (1):
├─ Acción: Juego activo, procesar input/colisiones
├─ Duración: Variable (hasta victoria)
├─ Siguiente: STATE_VICTORY o mantener

STATE_VICTORY (2):
├─ Acción: Animación de victoria
├─ Duración: 120 frames (~2 segundos)
├─ Siguiente: STATE_RETURNING

STATE_RETURNING (3):
├─ Acción: Fade a negro, preparar siguiente fase
├─ Duración: 60 frames (~1 segundo)
├─ Siguiente: Retornar a main.c
```

### 7.2 Timeline de Transición de Fases

```
TRANSICIÓN FASE 1 → FASE 2:

Evento: Victory en Fase 1
├─ Fade out música Polo (60 frames)
├─ Fade to black (60 frames)
├─ Cargar recursos Fase 2 (minigame_delivery_init)
├─ Fade in Fase 2 (60 frames)
├─ Reproducir música tejados (start XGM2_play)
└─ Estado: PLAYING

TRANSICIÓN FASE 2 → FASE 3:

Evento: Victory en Fase 2 (entregas >= 10)
├─ Reproducir snd_victoria (2 segundos)
├─ Mostrar "¡VICTORIA!" overlay (120 frames)
├─ Fade out música tejados (60 frames)
├─ Unload: Sprites/fondos Fase 2
├─ Load: Sprites/fondos Fase 3
├─ Fade in iglesia (60 frames)
├─ Reproducir música iglesia
└─ Continuar en minigameBells_init()

DURACIÓN TOTAL TRANSICIÓN: 
├─ Máximo: 360 frames (~6 segundos)
├─ Típico: 300 frames (~5 segundos)
└─ Mínimo: 180 frames (~3 segundos)
```

---

## <a name="estructura-datos"></a>

## 8. ESTRUCTURA DE DATOS C

### 8.1 Structs Principales

```c
/* ════════════════════════════════════════════════════════════════
   FILE: minigame_delivery.h
   ════════════════════════════════════════════════════════════════ */

#ifndef MINIGAME_DELIVERY_H
#define MINIGAME_DELIVERY_H

#include <genesis.h>
#include "gamecore.h"

/* ────────────────────────────────────────────────────────────────
   ESTRUCTURAS DE DATOS
   ──────────────────────────────────────────────────────────────── */

typedef struct {
    Sprite* sprite;
    s16 x, y;
    s8 velX, velY;
    u8 active;
    u16 rotation;
    u16 createdFrames;
} Regalo_Volador;

typedef struct {
    Sprite* sprite;
    s16 x, y;
    u8 active;          // 0=inactiva, 1=activa, 2=completada
    u8 frameParpadeo;   // contador para parpadeo tras entrega
    u8 spriteFrame;     // 0=apagada, 1=encendida
} Chimenea;

typedef struct {
    Sprite* sprite;
    s16 x;              // y siempre = 180
    s8 velX;
    u8 disparoFrame;    // contador para animación disparo
} Cannon_Tejado;

typedef struct {
    u32 elapsedFrames;
    u32 maxFrames;      // 5400 = 90 segundos
    u8 bloque;          // 0=facil, 1=media, 2=dificil
} FaseTimer;

/* ────────────────────────────────────────────────────────────────
   CONSTANTES
   ──────────────────────────────────────────────────────────────── */

#define NUM_REGALOS_VOL         6
#define NUM_CHIMENEAS          15
#define NUM_CHIMENEAS_ACTIVAS_MAX 10

#define OBJETIVO_ENTREGAS      10
#define CANNON_POS_Y          180
#define CANNON_MIN_X           32
#define CANNON_MAX_X          256
#define CANNON_SPEED           6
#define CANNON_FRICTION      0.85f

#define REGALO_INICIAL_VY     -4
#define REGALO_GRAVITY        0.3f
#define REGALO_MAX_VY          6
#define REGALO_SIZE_HALF      12

#define BULLET_COOLDOWN_FRAMES 25
#define CHIMENEA_WIDTH         20
#define CHIMENEA_HEIGHT        80
#define CHIMENEA_HITBOX_DIST   20

#define FASE_DURATION_FRAMES  5400     // 90 segundos
#define BLOQUE_1_FRAMES       1800     // 30 segundos
#define BLOQUE_2_FRAMES       3600     // 60 segundos

/* ────────────────────────────────────────────────────────────────
   FUNCIONES PÚBLICAS
   ──────────────────────────────────────────────────────────────── */

void minigameDelivery_init(void);
void minigameDelivery_update(void);
void minigameDelivery_render(void);
u8 minigameDelivery_isComplete(void);
void minigameDelivery_cleanup(void);

#endif /* MINIGAME_DELIVERY_H */
```

### 8.2 Variables Globales (minigame_delivery.c)

```c
/* ════════════════════════════════════════════════════════════════
   FILE: minigame_delivery.c - VARIABLES GLOBALES
   ════════════════════════════════════════════════════════════════ */

/* ────────────────────────────────────────────────────────────────
   RECURSOS GRÁFICOS
   ──────────────────────────────────────────────────────────────── */

static Map* mapFondoTejados;
static Map* mapNubesParalaje;

static Sprite* spriteCannon;
static Sprite* spriteRegalosActivos[NUM_REGALOS_VOL];

static u16 parallaxOffset;

/* ────────────────────────────────────────────────────────────────
   DATOS DE JUEGO
   ──────────────────────────────────────────────────────────────── */

// Regalos en vuelo
static Regalo_Volador regalosVoladores[NUM_REGALOS_VOL];
static u8 numRegalosActivos;

// Chimeneas
static Chimenea chimeneas[NUM_CHIMENEAS];
static u8 numChimeneaActivas;

// Cañón
static Cannon_Tejado cannon;
static u8 bulletCooldown;
static s8 cannonAccelX;

// Estados de juego
static u16 entregasCompletadas;         // 0-10
static u32 frameCounter;
static FaseTimer faseTimer;
static u8 minigameState;                // STATE_INIT, STATE_PLAYING, STATE_VICTORY

// Control
static u16 lastInput;
static u16 currentInput;

// Animación victoria
static u16 victoryAnimationFrames;

/* ────────────────────────────────────────────────────────────────
   DATOS DE FÍSICA
   ──────────────────────────────────────────────────────────────── */

static f32 windFactor;                  // Viento actual
static u8 dificultadMult;               // Multiplicador dificultad
```

---

## <a name="memoria"></a>

## 9. PRESUPUESTO DE MEMORIA

### 9.1 RAM de Fase 2

```
DESGLOSE DE MEMORIA RAM:

┌─ COMPONENTE                      │ BYTES  │ NOTAS                 │
├───────────────────────────────────┼────────┼───────────────────────┤
│ Array regalosVoladores[6]         │ 432    │ 72 bytes × 6          │
│ Array chimeneas[15]               │ 645    │ 43 bytes × 15         │
│ Estructura cannon                 │ 20     │ Cañón único           │
│ Variables globales (varias)       │ 200    │ Contadores, flags     │
│ Maps (fondos + paralaje)          │ 2048   │ Datos tilemap         │
│ Paletas cargadas                  │ 128    │ 64 colores × 2 pal    │
├───────────────────────────────────┼────────┼───────────────────────┤
│ TOTAL FASE 2                      │ 3473   │ ~3.4 KB               │
│ REMANENTE (64KB)                  │ 61063  │ ~60.6 KB (abundante)  │
└───────────────────────────────────┴────────┴───────────────────────┘
```

### 9.2 VRAM de Fase 2

```
DESGLOSE DE MEMORIA VRAM (64KB):

┌─ COMPONENTE                      │ BYTES  │ NOTAS                 │
├───────────────────────────────────┼────────┼───────────────────────┤
│ Tileset fondos tejados            │ 8192   │ Tiles comprimidos      │
│ Tileset nubes paralaje            │ 4096   │ Tiles overlay         │
│ Sprites loaded                    │ 4096   │ Bajo demanda          │
│ Paletas (PAL_COMMON)              │ 256    │ 16 colores × 4        │
│ Map buffer (fondos renderizados)  │ 2048   │ Buffer dinámico SGDK  │
├───────────────────────────────────┼────────┼───────────────────────┤
│ TOTAL VRAM USADO                  │ 18688  │ ~18.2 KB              │
│ REMANENTE (64KB)                  │ 45848  │ ~44.8 KB (bueno)      │
└───────────────────────────────────┴────────┴───────────────────────┘
```

### 9.3 Cartucho ROM

```
DESGLOSE ROM:

┌─ RECURSO                        │ TAMAÑO │ NOTAS                 │
├──────────────────────────────────┼────────┼───────────────────────┤
│ Código C compilado               │ ~50KB  │ Todo el juego         │
│ Fondos (2 TileSets comprimidos)  │ ~8 KB  │ LZ4H compression      │
│ Sprites (combinados)             │ ~20KB  │ Indexed 16-color      │
│ Música XGM2 (4 canciones)        │ ~16KB  │ VGZ comprimido        │
│ SFX PCM (8 efectos)              │ ~160KB │ 8-bit 8kHz WAV        │
│ Datos varios                     │ ~10KB  │ Strings, paletas      │
├──────────────────────────────────┼────────┼───────────────────────┤
│ TOTAL CARTUCHO                   │ ~264KB │ Dentro de límites      │
│ Límite máximo Mega Drive         │ 32MB   │ Cartuchos modernos     │
└──────────────────────────────────┴────────┴───────────────────────┘
```

---

## <a name="checklist"></a>

## 10. CHECKLIST DE IMPLEMENTACIÓN

### 10.1 Tareas Previas

- [ ] Crear archivos vacíos:
  - [ ] `inc/minigame_delivery.h`
  - [ ] `src/minigame_delivery.c`

- [ ] Crear assets gráficos:
  - [ ] `res/Sprites/GFX/Regalos_Pequeño.png` (24×24)
  - [ ] `res/Sprites/GFX/Chimenea.png` (20×160)
  - [ ] `res/Sprites/GFX/CanonTejado.png` (128×96)
  - [ ] `res/Backgrounds/FondoTejados.png` (512×256)
  - [ ] `res/Backgrounds/Nubes.png` (256×256)
  - [ ] `res/Palettes/FondoTejados.pal`

- [ ] Crear assets de audio:
  - [ ] `res/Audio/Sounds/snd_regalo_disparado.wav`
  - [ ] `res/Audio/Sounds/snd_entrega_exitosa.wav`
  - [ ] `res/Audio/Sounds/snd_chimenea_activa.wav`
  - [ ] Reusar `res/music/musica.vgm` (campanas) como pista temporal

- [ ] Actualizar resources_sprites.res / resources_sfx.res / resources_bg.res:
  ```makefile
  # Agregrar sprites Fase 2
  SPRITE sprite_regalo_entrega Regalos_Pequeño.png 3 3 BEST 1
  SPRITE sprite_chimenea Chimenea.png 1 2 BEST 1
  SPRITE sprite_canon_tejado CanonTejado.png 2 1 BEST 1
  SPRITE sprite_nube Nube.png 4 2 BEST 0
  
  # Agregar fondos
  TILESET imagefondotejadostile FondoTejados.png BEST
  MAP imagefondotejadosmap FondoTejados.png imagefondotejadostile BEST
  TILESET imagenubeparalajitile Nubes.png BEST
  MAP imagenubeparaljamap Nubes.png imagenubeparalajitile BEST
  PALETTE imagefondotejadospal FondoTejados.png
  
  # Agregar audio en resources_sfx.res
  WAV snd_regalo_disparado snd_regalo_disparado.wav XGM2
  WAV snd_entrega_exitosa snd_entrega_exitosa.wav XGM2
  WAV snd_chimenea_activa snd_chimenea_activa.wav XGM2
  ```

### 10.2 Implementación del Código

- [ ] **minigame_delivery.h**: Definir structs y funciones
  - [ ] typedef Regalo_Volador
  - [ ] typedef Chimenea
  - [ ] typedef Cannon_Tejado
  - [ ] typedef FaseTimer
  - [ ] Declarar funciones públicas

- [ ] **minigame_delivery.c**: Implementar funciones core
  - [ ] `minigameDelivery_init()` - Inicializar todo
    - [ ] Cargar maps fondos
    - [ ] Crear sprites cañón
    - [ ] Inicializar array chimeneas (15)
    - [ ] Reproducir música
  
  - [ ] `minigameDelivery_update()` - Lógica de juego
    - [ ] Leer entrada (LEFT/RIGHT/A)
    - [ ] Actualizar posición cañón
    - [ ] Procesar disparo (cooldown)
    - [ ] Crear regalo volador
    - [ ] Actualizar regalos voladores (física)
    - [ ] Actualizar parallax
    - [ ] Activar/desactivar chimeneas (por tiempo)
    - [ ] Detectar colisiones regalo-chimenea
    - [ ] Verificar victoria (entregas >= 10)
  
  - [ ] `minigameDelivery_render()` - Renderizado
    - [ ] Actualizar parallax BG_A
    - [ ] Renderizar sprites (SPR_update)
    - [ ] Actualizar HUD
    - [ ] Mostrar contador entregas
  
  - [ ] `minigameDelivery_isComplete()` - Verificar victoria
    - [ ] Retornar true si entregas >= 10

- [ ] **Funciones auxiliares**:
  - [ ] `inicializarChimeneas()` - Crear array con posiciones
  - [ ] `crearRegaloVolador()` - Pool de regalos
  - [ ] `liberarRegaloVolador()` - Reutilizar en pool
  - [ ] `actualizarChimeneasPorTiempo()` - Activación dinámica
  - [ ] `detectarEntrega()` - Colisión regalo-chimenea
  - [ ] `actualizarHUD()` - Mostrar contador
  - [ ] `onEntregaExitosa()` - Callback entrega
  - [ ] `actualizarParallax()` - Scroll nubes
  - [ ] `aplicarViento()` - Física viento
  - [ ] `getDificultadMultiplicador()` - Por bloque

### 10.3 Testing

- [ ] Compilación
  - [ ] `make clean`
  - [ ] `make rebuild`
  - [ ] ✓ Sin errores de compilación
  - [ ] ✓ Sin warnings críticos

- [ ] Funcionalidad básica
  - [ ] Fase 2 carga correctamente
  - [ ] Se ven fondos y sprites
  - [ ] Cañón aparece en pantalla
  - [ ] Se reproducen música/SFX

- [ ] Control
  - [ ] LEFT mueve cañón izquierda
  - [ ] RIGHT mueve cañón derecha
  - [ ] A dispara regalo (cada 25 frames)
  - [ ] Regalos aparecen en pantalla

- [ ] Física
  - [ ] Regalos caen con gravedad
  - [ ] Viento afecta trayectoria
  - [ ] Colisiones límites pantalla

- [ ] Chimeneas
  - [ ] Se ven 15 chimeneas
  - [ ] Se activan progresivamente
  - [ ] Cambian sprite al recibir regalo
  - [ ] Emiten SFX correcto

- [ ] Gameplay
  - [ ] Contador entregas incrementa
  - [ ] HUD actualiza correctamente
  - [ ] 10 entregas = Victoria
  - [ ] Transición a Fase 3

- [ ] Rendimiento
  - [ ] 60 FPS constante
  - [ ] Sin lag al disparar
  - [ ] Sin glitches visuales

### 10.4 Pulido y Optimización

- [ ] Audio
  - [ ] Música loop sin clic
  - [ ] SFX no solapan
  - [ ] Volumen balanceado

- [ ] Visualización
  - [ ] Colores bien asignados
  - [ ] Sprites sin artefactos
  - [ ] Parallax suave

- [ ] Transiciones
  - [ ] Fade in/out suave
  - [ ] Cambio música sincronizado
  - [ ] Paso a Fase 3 automático

---

## 📊 RESUMEN EJECUTIVO FASE 2

```
ESPECIFICACIÓN COMPLETA: FASE 2 - ENTREGA (TEJADOS)

Nombre:          Entrega
Ubicación:       Tejados nocturnos, ciudad
Duración:        60-90 segundos (sin límite real)
Objetivo:        10 entregas de 20 regalos
Dificultad:      Media
Mecánica:        Precisión + Timing

COMPONENTES:
├─ Sprites:      4 types (regalo, chimenea, cañón, nube)
├─ Fondos:       2 layers (base + paralaje)
├─ Música:       1 loop XGM2
├─ SFX:           3 efectos PCM
└─ Total:        ~3.4 KB RAM, ~18 KB VRAM, ~264 KB ROM

ESTADO:          📋 LISTA PARA IMPLEMENTACIÓN
ESTIMADO:        4-6 horas de desarrollo
```

---

**Especificación Técnica - Fase 2: Entrega (Tejados)**
**Documento Completo y Exhaustivo**
**Diciembre 2025**
