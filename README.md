# 🎄 JUEGO NAVIDAD 2026 - SEGA GENESIS

Juego minijuegos navideño para Sega Genesis/Mega Drive desarrollado con SGDK.

## 📋 Requisitos

- SGDK (Sega Genesis Development Kit)
- GCC m68k
- Bash (para scripts)
- Emulador Blastem (opcional para testing)

## 🚀 Compilar

```bash
make           # Compilar
make run       # Compilar y ejecutar
make clean     # Limpiar intermedios
make rebuild   # Limpiar y compilar todo
```

## 📁 Estructura

```
christmas-game-2026/
├── src/              (Archivos C)
├── inc/              (Headers)
├── res/gfx/sprites/  (Sprites PNG)
├── res/sfx/          (Audio)
├── build/            (Output: rom.bin)
├── Makefile
└── README.md
```

## 🎮 Fases del Juego

- **Fase 1**: Recogida (Polo Norte) - PLACEHOLDER
- **Fase 2**: Entrega (Tejados) - PLACEHOLDER
- **Fase 3**: Campanadas (Iglesia) - ⭐ ACTUAL
- **Fase 4**: Celebración (Fiesta) - PLACEHOLDER

## 👨‍💻 Desarrollo

Basado en código FELIZ 2025 adaptado para estructura modular:
- game_core.c: Funciones base reutilizables
- minigame_*.c: Cada minijuego independiente

## 📝 Notas

- Rendimiento: 60 FPS
- Resolución: 320×224
- Paletas: 4 (reutilizables)
- Memoria: ~64KB RAM
