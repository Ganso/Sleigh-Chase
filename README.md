# Feliz 2026 (Mega Drive) by Geesebumps

Juego de minijuegos navidenos para Sega Genesis/Mega Drive desarrollado con SGDK.

## Estado actual
- **Fase 1**: Recogida (Polo Norte) - ¡Completada a nivel mecánicas! Scroll vertical, inercia completa, recogida de regalos, marcas de elfos, robo de enemigos y gestión de HUD/música. Pendientes solo ajustes de arte/SFX finales si se desea.
- **Fase 2**: Entrega (Tejados) - Base jugable creada: fondo de tejados con nieve, movimiento libre, chimeneas 48x48 en posiciones fijas y entrega manual de regalos (botón A) con contador decreciente desde 10. Enemigos placeholder reutilizados de los elfos con reinicio al contacto.
- **Fase 3**: Campanadas (Iglesia) - Implementada: fondo + paralaje de nieve, canion lateral con cooldown, campanas/bombas con SFX y HUD propio.
- **Fase 4**: Celebracion (Fiesta) - Placeholder: contador simple hasta 300 frames.

## Requisitos
- SGDK (Sega Genesis Development Kit) configurado en `SGDK_PATH` o `%GDK%`.
- Toolchain m68k incluida en SGDK; la compilacion se realiza solo en local.

## Estructura
- `src/`: codigo C de fases y sistemas (core, audio, HUD, nieve, intro).
- `inc/`: headers correspondientes.
- `res/`: definiciones `.res` y recursos generados (`resources_*.h`, `res_geesebumps.h`).
- `documentos/`: documentacion del proyecto y referencia SGDK (`documentos/sgdk-reference-2025-11-15.txt`).
- `out/`: binarios generados localmente (ROM y objetos); no se editan a mano.

## Compilar y ejecutar (solo en local)
- En raiz: `make` usa `SGDK_PATH` y genera `build/rom.bin`.
- VS Code: tareas `make`, `clean` y `run-emulator` en `.vscode/tasks.json` (usa `%GDK%\\bin\\make` y un script Blastem).
- No ejecutes compilacion desde IA/CI; solo el equipo humano con SGDK instalado.

## Notas de desarrollo
- El bucle principal (`src/main.c`) orquesta `INTRO -> PICKUP -> DELIVERY -> BELLS -> CELEBRATION -> END`, aplicando `gameCore_fadeToBlack()` entre fases.
- Recursos nuevos se anaden editando los `.res` y regenerando con `rescomp`; no modificar a mano los headers generados.
- Mantener este README actualizado cuando cambie el estado de las fases, dependencias o estructura.
