# Guia rapida para IA en Feliz 2026

Notas para cualquier asistente que edite el proyecto. Mantener este resumen visible antes de proponer cambios de codigo.

## Normas generales
- No compiles ni lances `make`/tareas: la ROM se genera solo en local con SGDK instalado (`%GDK%` / `SGDK_PATH`). El emulador se ejecuta localmente (Blastem) y la carpeta `out/` contiene binarios generados que no se deben tocar.
- Mantener `README.md` siempre actualizado ante cualquier cambio funcional o de estructura.
- Trabaja en ASCII siempre que sea posible y evita anadir caracteres especiales si no son imprescindibles.
- No reescribas ficheros generados por SGDK (`res/*.h`, `resources_*.h`, `res_geesebumps.h`). Si hacen falta nuevos assets, se editan los `.res` y se regeneran localmente.
- Respeta la estructura de minijuegos: cada fase expone `*_init`, `*_update`, `*_render`, `*_isComplete` y el bucle principal orquesta el avance de fase.
- Usa el helper de logging de SGDK (`KLog`, `kprintf`, `KDebug_AlertNumber`) para depurar en emulador; no introduzcas `printf` estandar.

## Flujo y arquitectura
- `src/main.c` es el orquestador: fases `INTRO -> PICKUP -> DELIVERY -> BELLS -> CELEBRATION -> END`. Tras cada `*_isComplete()` se aplica `gameCore_fadeToBlack()` antes de avanzar.
- Motor comun (`game_core.*`): lectura unificada de input (`gameCore_readInput`), timers (`GameTimer`), reinicio del indice global de tiles (`gameCore_resetTileIndex`), fade combinado musica+paletas (`gameCore_fadeToBlack`). Inercia compartida: usa `GameInertia` y los helpers `gameCore_applyInertiaAxis/Movement` (parametrizable por fase). `globalTileIndex` debe avanzarse al cargar tiles para evitar solapes en VRAM.
- HUD basico (`hud.*`): texto en BG con `VDP_drawText` para contadores por fase. Fase 3 usa su propio HUD de campanas; resto puede reutilizar `hud_*`.
- Audio central (`audio_manager.*`): `audio_init` configura volumenes y `audio_play_phaseX` dispara las pistas (`XGM2_play`). Usa `audio_stop_music` al salir.
- Cutscenes (`cutscene.*`): antes de cada fase se limpia audio y sprites, se dibuja `image_fondo_cutscene` y se muestran textos letra a letra antes de llamar al siguiente `*_init`.
- Efecto de nieve (`snow_effect.*`): carga `image_primer_plano_nieve` en `BG_A` usando el `globalTileIndex` que se le pasa por puntero; se debe llamar tras cargar el fondo para mantener el orden de tiles.
- Importante: Después de hacer el primer ScrollTo tras inicializar un MAP, y antes de hacer el siguiente, es importante hacer un refresco de pantalla con SYS_doVBlankProcess();
- Es importante que todos los fondos se puedan organizar en bloques de 128x128.
- Documenta todas las inicializaciones de variables en C con un comentario breve de propósito usando formato compatible con Doxygen (`/**< ... */`).

## Detalle por fase (src/)
- Fase 1 `minigame_pickup.c`: vista cenital con pista desplazada hacia abajo (scroll en `BG_B`). Limites jugables a 10% de cada lateral; movimiento con inercia (`applyInertiaMovement`) y hitbox reducida del trineo. Faltan el disparo especial definitivo y el sprite del regalo que lanzan los elfos laterales. `giftsCollected` sube al chocar con arboles/elfos; el fondo se desplaza con `trackOffsetY` normalizado y el overlay de nieve se mueve con `snowEffect_update`.
- Fase 2 `minigame_delivery.c`: fase jugable con chimeneas activas, enemigos y nieve compartida. El mapa de tejados (`resources_bg.h`) se carga en BG, Santa mueve su trineo con inercia y suelta regalos con cooldown (`DROP_COOLDOWN_FRAMES`). `snow_effect` se puede reutilizar tal cual y los contadores gráficos se renderizan con sprites dedicados.
- Fase 3 `minigame_bells.c`: fase completa. Fondo en `BG_B` + nieve en `BG_A`; canion lateral con inercia y cooldown de disparo (`BULLET_COOLDOWN_FRAMES`). Arreglos: `bells` (caida con parpadeo al llegar abajo), `bombs` (resetean progreso y parpadean todas), `fixedBells` muestran progreso y cambian a color al acertar. `fireBullet` crea sprites de confeti y `detectarColisionesBala` decide impacto con campana o bomba (reproduce SFX desde `resources_sfx.h`).
- Fase 4 `minigame_celebration.c`: placeholder temporal; solo cuenta frames (`DURACION_CELEBRACION`) y llama a `SPR_update`/`SYS_doVBlankProcess`.
- Intro `geesebumps.c`: muestra logo con fades y musica `music_geesebumps`; usa `SPR_addSpriteSafe` y eventos de joystick para saltar.

## Recursos y assets
- Cabeceras `resources_bg.h`, `resources_sprites.h`, `resources_sfx.h`, `resources_music.h`, `res_geesebumps.h` se generan con `rescomp` a partir de `.res` en `res/`. No editarlas a mano.
- Cuando cargues un tileset/mapa: usa el `globalTileIndex` actual y despues incrementalo con `numTile`. Ejemplo en fase 3 al cargar `image_fondo` y luego el overlay de nieve.
- Paletas: se asignan a slots fijos (`PAL_COMMON`, `PAL_PLAYER`, `PAL_ENEMY`, `PAL_EFFECT` en `game_core.h`). Respeta esos bancos para evitar sorpresas con sprites/fondos.

## Compilacion (solo referencia, no ejecutar)
- Makefile raiz usa `SGDK_PATH` y las toolchains `m68k-elf-*`; genera `build/rom.bin`. En VS Code hay tareas que llaman a `%GDK%\\bin\\make -f %GDK%\\makefile.gen` y un script `run-emulator` para Blastem. Todo esto se ejecuta solo en local por el equipo humano.

## Documentacion disponible
- Referencia completa de SGDK: `documentos/sgdk-reference-2025-11-15.txt`.
- Otros docs del proyecto: `documentos/DIAGRAMA_TECNICO.md`, `documentos/ESPECIFICACION_COMPLETA.md`, `documentos/FASE_2_TEJADOS.md`, `documentos/RESUMEN_EJECUTIVO.md`.
