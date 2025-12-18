# ✅ ESPECIFICACIÓN MECÁNICA - FASE 1: RECOGIDA (POLO NORTE)

## 1. CONTEXTO Y OBJETIVO
- **Vista**: Cenital con scroll vertical continuo sobre pista nevada circular.
- **Meta**: Reunir **10 regalos** antes de que los enemigos roben la carga o se agote el HUD de inventario.
- **Ritmo**: Arcade con inercia completa; la velocidad de scroll oscila entre 0.5 y 1.5 px/frame para simular tobogán.

## 2. CONTROLES Y FLUJO
- **Dirección**: D-Pad para mover el trineo (Santa) en 8 direcciones dentro de los márgenes jugables.
- **Especial (BOTÓN B)**: Al acumular 3 regalos se carga un despeje que elimina enemigos activos y reinicia su IA.
- **Entrada leída**: `gameCore_readInput()` mapea `BUTTON_LEFT/RIGHT/UP/DOWN/B` con inercia aplicada vía `gameCore_applyInertiaMovement()`.

## 3. SISTEMA DE ENTIDADES
- **Santa**: Sprite 80×128 con hitbox reducida; velocidades `vx/vy` amortiguadas por fricción. Parpadea tras choque con árbol.
- **Árboles**: 2 obstáculos máximos; hitbox estrecha en la copa (offset de 8 px) que provoca invulnerabilidad temporal y reset de enemigos.
- **Elfos**: 4 receptores; aparecen en laterales con marcas flotantes y sombras. Cada elfo acepta un regalo parabólico y luego se reprograma su respawn.
- **Enemigos**: Hasta 3 concurrentes; persiguen lateralmente, roban un regalo y huyen con velocidad extra. El despeje especial los elimina.
- **Regalos**: Proyectiles parabólicos (`GIFT_ARC_HEIGHT=50`) que aterrizan junto al elfo; el HUD registra entregas y pérdidas con parpadeo.

## 4. HUD, AUDIO Y PROGRESO
- **Contador**: Dos filas de 5 iconos (`GIFT_COUNTER_MAX=10`), con parpadeo cuando se pierden regalos (`GIFT_LOSS_MAX_OFFSET=2`).
- **Música**: `audio_play_phase1()` arranca tras 40 frames de espera (grito "Ho Ho Ho" previo); volúmenes FM=70, PSG=100.
- **Nieve**: `snowEffect` corre en plano independiente y comparte paleta con fondo.
- **Victoria/Transición**: Al alcanzar 10 regalos (`TARGET_GIFTS`) se marca `phaseChangeRequested` y el núcleo pasa a Fase 2.

## 5. CHECKLIST DE IMPLEMENTACIÓN
- [x] Scroll vertical y bucle de mapa (`TRACK_LOOP_PX=512`).
- [x] Inercia completa en X/Y y límites laterales (`FORBIDDEN_PERCENT=10`).
- [x] Spawns escalonados: 1 enemigo activo inicial, árbol extra tras 7 regalos.
- [x] Robo de enemigos con ruta de fuga (`ENEMY_ESCAPE_SPEED=3`).
- [x] HUD dual y efecto blink en colisiones/pérdidas.
- [ ] Ajustes finales de arte/SFX opcionales.
