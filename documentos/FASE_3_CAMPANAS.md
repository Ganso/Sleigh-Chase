# ✅ ESPECIFICACIÓN MECÁNICA - FASE 3: CAMPANADAS (IGLESIA)

## 1. CONTEXTO Y OBJETIVO
- **Vista**: Lateral fijo con plano nevado y cañón en la base de la pantalla.
- **Meta**: Encender **12 campanillas** del marcador inferior y formar el mensaje **FELIZ2026** disparando letras en orden.
- **Ritmo**: Dos subfases enlazadas (campanas → letras) con control de tiempo mediante `GameTimer`.

## 2. CONTROLES Y FÍSICA
- **Dirección**: D-Pad izquierda/derecha mueve el cañón con inercia (`CANNON_ACCEL=1`, `CANNON_FRICTION=1`, `CANNON_MAX_VEL=6`).
- **Disparo (BOTÓN A)**: Lanza una bala vertical; cooldown de 30 frames y animación de retroceso.
- **Gestión interna**: `gameCore_applyInertiaAxis()` limita el movimiento al ancho de pantalla y reinicia la animación cuando termina el recoil.

## 3. ENTIDADES Y FLUJO DE JUEGO
- **Campanas móviles**: 6 caídas simultáneas con parpadeo previo a desaparecer; cada acierto enciende una campanilla fija.
- **Campanillas fijas**: 12 iconos en el HUD inferior; cambian a color al acertar y reinician a blanco y negro si explota una bomba.
- **Bombas**: 3 concurrentes; al impactarlas parpadean todas las entidades, se resetea el progreso de la subfase activa y se reproduce `snd_bomba`.
- **Letras**: 8 sprites descendentes reutilizados para formar `FELIZ2026`; el índice correcto se calcula dinámicamente (`getTargetLetterIndex`).
- **Bala**: Hasta 3 simultáneas, recicladas tras salir de pantalla o colisionar.

## 4. HUD, AUDIO Y TRANSICIÓN
- **Indicadores inferiores**: Palabra `FELIZ2026` en blanco y negro que se colorea letra a letra; parpadeo gestionado en `updateFeliz2025Blink()`.
- **Música y SFX**: `audio_play_phase3()` en loop; impactos usan `snd_campana` y `snd_bomba` con PCM.
- **Nieve**: `snowEffect` mantiene el parallax activo mientras dura la fase.
- **Victoria**: Tras colorear las 9 letras objetivo se marca `victoryTriggered`, `currentPhase=PHASE_COMPLETED` y el núcleo puede saltar a la celebración.

## 5. CHECKLIST DE IMPLEMENTACIÓN
- [x] Subfases secuenciales con cambio automático tras 12 campanas (`startLettersPhase`).
- [x] Cooldown de disparo y animación de cañón sincronizada.
- [x] Reseteo de progreso al detonar bombas tanto en campanas como en letras.
- [x] Timer global activo para derrota si se desea (expuesto en `gameTimer`).
- [ ] Ajustes finales de paletas/arte opcionales.
