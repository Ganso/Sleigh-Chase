# ═══════════════════════════════════════════════════════════════════════════
# Makefile: Juego Navidad 2026 - Sega Genesis/Mega Drive
# ═══════════════════════════════════════════════════════════════════════════

TARGET = rom.bin
LIBMD = $(SGDK_PATH)/lib
INC = -I$(SGDK_PATH)/inc -Iinc
CC = m68k-elf-gcc
LD = m68k-elf-ld
AS = m68k-elf-as
AR = m68k-elf-ar
OBJCOPY = m68k-elf-objcopy
CFLAGS = -m68000 -Wall -O2 -fomit-frame-pointer $(INC)
ASFLAGS = -m68000
LDFLAGS = -T $(LIBMD)/genesis.ld

# Archivos fuente
SOURCES = src/main.c \
          src/game_core.c \
          src/audio_manager.c \
          src/hud.c \
          src/snow_effect.c \
          src/geesebumps.c \
          src/minigame_bells.c \
          src/minigame_pickup.c \
          src/minigame_delivery.c \
          src/minigame_celebration.c

OBJECTS = $(SOURCES:.c=.o)

# Compilación principal
$(TARGET): $(OBJECTS)
	@echo "[LD] Enlazando $@..."
	$(LD) $(LDFLAGS) -o build/$@ $(OBJECTS) $(LIBMD)/libmd.a
	@echo "✅ Compilación exitosa: build/$@"

%.o: %.c
	@echo "[CC] Compilando $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar
clean:
	@echo "[CLEAN] Eliminando archivos intermedios..."
	rm -f $(OBJECTS) build/$(TARGET)
	@echo "✅ Limpieza completada"

# Recompilar completamente
rebuild: clean $(TARGET)

# Ejecutar en emulador
run: $(TARGET)
	@echo "[RUN] Abriendo en emulador..."
	blastem build/$(TARGET) 2>/dev/null || echo "💡 Instala Blastem: https://www.retrodev.com/blastem/"

# Información del proyecto
info:
	@echo "═══════════════════════════════════════════════════════════════════════════"
	@echo "JUEGO NAVIDAD 2026 - SEGA GENESIS"
	@echo "═══════════════════════════════════════════════════════════════════════════"
	@echo "Proyecto: $(TARGET)"
	@echo "SGDK: $(SGDK_PATH)"
	@echo "Archivos C: $(SOURCES)"
	@echo ""
	@echo "Comandos disponibles:"
	@echo "  make           - Compilar"
	@echo "  make clean     - Limpiar intermedios"
	@echo "  make rebuild   - Limpiar y compilar"
	@echo "  make run       - Compilar y ejecutar"
	@echo "  make info      - Mostrar esta información"
	@echo "═══════════════════════════════════════════════════════════════════════════"

.PHONY: clean rebuild run info
