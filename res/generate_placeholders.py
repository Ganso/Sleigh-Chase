from pathlib import Path
import math
import random
import wave

from PIL import Image, ImageDraw

BASE_DIR = Path(__file__).resolve().parent
RESOURCES_FILE = BASE_DIR / "resources.res"

PALETTE = [
    (0, 0, 0),
    (255, 0, 0),
    (0, 255, 0),
    (0, 0, 255),
    (255, 255, 0),
    (255, 0, 255),
    (0, 255, 255),
    (255, 128, 0),
    (128, 0, 255),
    (0, 128, 255),
    (128, 255, 0),
    (255, 0, 128),
    (0, 255, 128),
    (128, 0, 128),
    (128, 128, 0),
    (128, 128, 128),
]

ENTRIES = [
    {
        "path": "bg/FondoPolo.png",
        "resource_line": 'PALETTE image_fondo_polo_pal "bg/FondoPolo.png" BEST # Placeholder (Paleta fondo polar fase uno)',
        "width": 128,
        "height": 96,
    },
    {
        "path": "bg/FondoTejados.png",
        "resource_line": 'PALETTE image_fondo_tejados_pal "bg/FondoTejados.png" BEST # Placeholder (Paleta fondo tejados noche navidad)',
        "width": 128,
        "height": 96,
    },
    {
        "path": "bg/FondoFiesta.png",
        "resource_line": 'PALETTE image_fondo_fiesta_pal "bg/FondoFiesta.png" BEST # Placeholder (Paleta fondo fiesta final colorido)',
        "width": 128,
        "height": 96,
    },
    {
        "path": "sprites/Regalo.png",
        "resource_line": 'SPRITE sprite_regalo "sprites/Regalo.png" 4 4 BEST 1 # Placeholder (Regalo cayendo en fase polar)',
        "width": 64,
        "height": 64,
    },
    {
        "path": "sprites/RegaloBN.png",
        "resource_line": 'SPRITE sprite_regalo_bn "sprites/RegaloBN.png" 4 4 BEST 1 # Placeholder (Contador regalo blanco y negro)',
        "width": 64,
        "height": 64,
    },
    {
        "path": "sprites/CanonPolo.png",
        "resource_line": 'SPRITE sprite_canon_polo "sprites/CanonPolo.png" 10 8 BEST 1 # Placeholder (Canon trineo fase polo principal)',
        "width": 64,
        "height": 64,
    },
    {
        "path": "sprites/RedConfeti.png",
        "resource_line": 'SPRITE sprite_red_confeti "sprites/RedConfeti.png" 1 1 BEST 2 # Placeholder (Disparo red confeti polo rapido)',
        "width": 32,
        "height": 32,
    },
    {
        "path": "sprites/CopoGrande.png",
        "resource_line": 'SPRITE sprite_copo_grande "sprites/CopoGrande.png" 8 8 BEST # Placeholder (Obstaculo copo nieve grande estatico)',
        "width": 64,
        "height": 64,
    },
    {
        "path": "sprites/Regalos_Pequeno.png",
        "resource_line": 'SPRITE sprite_regalo_entrega "sprites/Regalos_Pequeno.png" 3 3 BEST 1 # Placeholder (Regalo pequeño entrega tejados volador)',
        "width": 48,
        "height": 48,
    },
    {
        "path": "sprites/Chimenea.png",
        "resource_line": 'SPRITE sprite_chimenea "sprites/Chimenea.png" 3 10 BEST 1 # Placeholder (Chimenea activable con regalo encendida)',
        "width": 48,
        "height": 96,
    },
    {
        "path": "sprites/CanonTejado.png",
        "resource_line": 'SPRITE sprite_canon_tejado "sprites/CanonTejado.png" 8 12 BEST 1 # Placeholder (Canon tejado entrega regalos principal)',
        "width": 64,
        "height": 96,
    },
    {
        "path": "sprites/Nube.png",
        "resource_line": 'SPRITE sprite_nube "sprites/Nube.png" 8 4 BEST # Placeholder (Nube decorativa paralaje tejados suave)',
        "width": 96,
        "height": 48,
    },
    {
        "path": "sprites/Confeti.png",
        "resource_line": 'SPRITE sprite_confeti "sprites/Confeti.png" 1 1 BEST 2 # Placeholder (Particula confeti celebracion final colorido)',
        "width": 32,
        "height": 32,
    },
    {
        "path": "sprites/PersonajeFiesta.png",
        "resource_line": 'SPRITE sprite_personaje "sprites/PersonajeFiesta.png" 8 8 BEST 1 # Placeholder (Personaje bailando fiesta final animado)',
        "width": 64,
        "height": 64,
    },
    {
        "path": "sprites/ArbolFiesta.png",
        "resource_line": 'SPRITE sprite_arbol_fiesta "sprites/ArbolFiesta.png" 10 15 BEST 1 # Placeholder (Arbol navidad animado fiesta central)',
        "width": 96,
        "height": 120,
    },
    {
        "path": "sprites/Globo.png",
        "resource_line": 'SPRITE sprite_globo "sprites/Globo.png" 3 4 BEST 2 # Placeholder (Globo decoracion fiesta final colorido)',
        "width": 48,
        "height": 64,
    },
    {
        "path": "bg/FondoPolo.png",
        "resource_line": 'TILESET image_fondo_polo_tile "bg/FondoPolo.png" BEST # Placeholder (Fondo tileset fase polo principal)',
        "width": 128,
        "height": 96,
    },
    {
        "path": "bg/FondoPolo.png",
        "resource_line": 'MAP image_fondo_polo_map "bg/FondoPolo.png" image_fondo_polo_tile BEST # Placeholder (Mapa fondo fase polo principal)',
        "width": 128,
        "height": 96,
    },
    {
        "path": "bg/CoposFondo.png",
        "resource_line": 'TILESET image_copos_fondo_tile "bg/CoposFondo.png" BEST # Placeholder (Capa copos paralaje polo fondo)',
        "width": 128,
        "height": 96,
    },
    {
        "path": "bg/CoposFondo.png",
        "resource_line": 'MAP image_copos_fondo_map "bg/CoposFondo.png" image_copos_fondo_tile BEST # Placeholder (Mapa copos paralaje polo fondo)',
        "width": 128,
        "height": 96,
    },
    {
        "path": "bg/FondoTejados.png",
        "resource_line": 'TILESET image_fondo_tejados_tile "bg/FondoTejados.png" BEST # Placeholder (Fondo base fase tejados principal)',
        "width": 128,
        "height": 96,
    },
    {
        "path": "bg/FondoTejados.png",
        "resource_line": 'MAP image_fondo_tejados_map "bg/FondoTejados.png" image_fondo_tejados_tile BEST # Placeholder (Mapa fondo fase tejados principal)',
        "width": 128,
        "height": 96,
    },
    {
        "path": "bg/Nubes.png",
        "resource_line": 'TILESET image_nubes_paralaje_tile "bg/Nubes.png" BEST # Placeholder (Nubes paralaje fase tejados decorativas)',
        "width": 128,
        "height": 96,
    },
    {
        "path": "bg/Nubes.png",
        "resource_line": 'MAP image_nubes_paralaje_map "bg/Nubes.png" image_nubes_paralaje_tile BEST # Placeholder (Mapa nubes paralaje tejados decorativas)',
        "width": 128,
        "height": 96,
    },
    {
        "path": "bg/FondoFiesta.png",
        "resource_line": 'TILESET image_fondo_fiesta_tile "bg/FondoFiesta.png" BEST # Placeholder (Fondo sala fiesta final principal)',
        "width": 128,
        "height": 96,
    },
    {
        "path": "bg/FondoFiesta.png",
        "resource_line": 'MAP image_fondo_fiesta_map "bg/FondoFiesta.png" image_fondo_fiesta_tile BEST # Placeholder (Mapa fondo fiesta final principal)',
        "width": 128,
        "height": 96,
    },
    {
        "path": "bg/LucesParpadeo.png",
        "resource_line": 'TILESET image_luces_parpadeo_tile "bg/LucesParpadeo.png" BEST # Placeholder (Capa luces parpadeo fiesta animada)',
        "width": 128,
        "height": 96,
    },
    {
        "path": "bg/LucesParpadeo.png",
        "resource_line": 'MAP image_luces_parpadeo_map "bg/LucesParpadeo.png" image_luces_parpadeo_tile BEST # Placeholder (Mapa luces parpadeo fiesta animada)',
        "width": 128,
        "height": 96,
    },
    {
        "path": "sfx/snd_regalo_recogido.wav",
        "resource_line": 'WAV snd_regalo_recogido "sfx/snd_regalo_recogido.wav" XGM2 # Placeholder (Sonido recoger regalo polo positivo)',
    },
    {
        "path": "sfx/snd_disparo_red.wav",
        "resource_line": 'WAV snd_disparo_red "sfx/snd_disparo_red.wav" XGM2 # Placeholder (Disparo red confeti polo rapido)',
    },
    {
        "path": "sfx/snd_obstaculo_golpe.wav",
        "resource_line": 'WAV snd_obstaculo_golpe "sfx/snd_obstaculo_golpe.wav" XGM2 # Placeholder (Golpe contra copo obstaculo suave)',
    },
    {
        "path": "sfx/snd_regalo_disparado.wav",
        "resource_line": 'WAV snd_regalo_disparado "sfx/snd_regalo_disparado.wav" XGM2 # Placeholder (Lanzamiento regalo desde canon tejados)',
    },
    {
        "path": "sfx/snd_entrega_exitosa.wav",
        "resource_line": 'WAV snd_entrega_exitosa "sfx/snd_entrega_exitosa.wav" XGM2 # Placeholder (Regalo entregado en chimenea exitoso)',
    },
    {
        "path": "sfx/snd_chimenea_activa.wav",
        "resource_line": 'WAV snd_chimenea_activa "sfx/snd_chimenea_activa.wav" XGM2 # Placeholder (Chimenea encendida tras regalo victoria)',
    },
    {
        "path": "sfx/snd_confeti_choque.wav",
        "resource_line": 'WAV snd_confeti_choque "sfx/snd_confeti_choque.wav" XGM2 # Placeholder (Choque confeti celebracion final alegre)',
    },
    {
        "path": "music/musica_polo.wav",
        "resource_line": 'WAV musica_polo "music/musica_polo.wav" XGM2 # Placeholder (Musica ambiente fase polo inicial)',
    },
    {
        "path": "music/musica_tejados.wav",
        "resource_line": 'WAV musica_tejados "music/musica_tejados.wav" XGM2 # Placeholder (Musica ambiente fase tejados nocturna)',
    },
    {
        "path": "music/musica_celebracion.wav",
        "resource_line": 'WAV musica_celebracion "music/musica_celebracion.wav" XGM2 # Placeholder (Musica ambiente fase fiesta celebracion)',
    },
]


def pick_color_index(seed: str) -> int:
    rng = random.Random(seed)
    return rng.randint(1, 15)


def generate_image(path: Path, width: int, height: int, seed: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    image = Image.new("P", (width, height), color=0)
    image.putpalette([component for rgb in PALETTE for component in rgb])
    draw = ImageDraw.Draw(image)

    color_index = pick_color_index(seed)
    radius = min(width, height) // 3
    center = (width // 2, height // 2)
    bbox = [
        center[0] - radius,
        center[1] - radius,
        center[0] + radius,
        center[1] + radius,
    ]
    draw.ellipse(bbox, fill=color_index)
    image.save(path)


def generate_beep(path: Path, duration: float = 0.25, sample_rate: int = 22050) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    freq = random.randint(440, 1320)
    total_frames = int(duration * sample_rate)

    with wave.open(str(path), "w") as wav_file:
        wav_file.setnchannels(1)
        wav_file.setsampwidth(2)
        wav_file.setframerate(sample_rate)

        frames = bytearray()
        for i in range(total_frames):
            amplitude = 16000
            value = int(amplitude * math.sin(2 * math.pi * freq * (i / sample_rate)))
            frames.extend(value.to_bytes(2, byteorder="little", signed=True))

        wav_file.writeframes(frames)


def ensure_resources(entries: list[dict]) -> None:
    existing_lines = []
    if RESOURCES_FILE.exists():
        existing_lines = RESOURCES_FILE.read_text(encoding="utf-8").splitlines()
    known = {line.strip() for line in existing_lines}
    updated = list(existing_lines)

    for entry in entries:
        normalized = entry["resource_line"].strip()
        if normalized not in known:
            updated.append(entry["resource_line"])
            known.add(normalized)

    if updated != existing_lines:
        RESOURCES_FILE.write_text("\n".join(updated) + "\n", encoding="utf-8")


def ensure_files(entries: list[dict]) -> None:
    for entry in entries:
        path = BASE_DIR / entry["path"]
        if path.exists():
            continue

        if path.suffix.lower() == ".wav":
            generate_beep(path)
        else:
            width = entry.get("width", 64)
            height = entry.get("height", 64)
            generate_image(path, width, height, seed=entry["path"])


def main() -> None:
    ensure_files(ENTRIES)
    ensure_resources(ENTRIES)


if __name__ == "__main__":
    main()
