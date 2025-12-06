from pathlib import Path
import math
import random
import wave

from PIL import Image, ImageDraw

BASE_DIR = Path(__file__).resolve().parent
RESOURCES_FILE = BASE_DIR / "resources.res"
DEFAULT_AUDIO_DURATION = 0.25

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
        "width": 512,
        "height": 256,
        "final_size": "320x224 píxeles (tileset 512x256)",
        "usage": "Paleta base del escenario Polo Norte con nieve, cielo azul y luces frías",
        "palette": "PAL_COMMON con blancos, azules fríos y negros",
    },
    {
        "path": "bg/PistaPolo.png",
        "resource_line": 'PALETTE image_pista_polo_pal "bg/PistaPolo.png" BEST # Placeholder (Paleta pista scroll fase uno)',
        "width": 320,
        "height": 448,
        "final_size": "Mapa 320x448 (dos pantallas de alto en bucle)",
        "usage": "Paleta dedicada a la pista vertical con vallas laterales y nieve",
        "palette": "PAL_COMMON con blancos y marcas de pista en azul",
    },
    {
        "path": "bg/FondoTejados.png",
        "resource_line": 'PALETTE image_fondo_tejados_pal "bg/FondoTejados.png" BEST # Placeholder (Paleta fondo tejados noche navidad)',
        "width": 512,
        "height": 256,
        "final_size": "320x224 píxeles (tileset 512x256)",
        "usage": "Paleta nocturna de la ciudad con tejados, luna y luces amarillas",
        "palette": "PAL_COMMON con azules oscuros, grises, rojos y amarillos de ventanas",
    },
    {
        "path": "bg/FondoFiesta.png",
        "resource_line": 'PALETTE image_fondo_fiesta_pal "bg/FondoFiesta.png" BEST # Placeholder (Paleta fondo fiesta final colorido)',
        "width": 512,
        "height": 256,
        "final_size": "320x224 píxeles (tileset 512x256)",
        "usage": "Paleta cálida para el salón decorado de la fase de celebración",
        "palette": "PAL_COMMON con rojos, dorados y verdes festivos",
    },
    {
        "path": "sprites/Regalo.png",
        "resource_line": 'SPRITE sprite_regalo "sprites/Regalo.png" 4 4 BEST 1 # Placeholder (Regalo cayendo en fase polar)',
        "width": 32,
        "height": 32,
        "usage": "Regalo que cae en la fase 1; el jugador debe recogerlo",
        "frames": 1,
        "palette": "PAL_PLAYER con rojo brillante y cinta dorada",
        "visual": "Caja cúbica roja con lazo dorado vista frontal",
    },
    {
        "path": "sprites/RegaloBN.png",
        "resource_line": 'SPRITE sprite_regalo_bn "sprites/RegaloBN.png" 4 4 BEST 1 # Placeholder (Contador regalo blanco y negro)',
        "width": 32,
        "height": 32,
        "usage": "Versión en escala de grises para el contador de regalos",
        "frames": 1,
        "palette": "PAL_PLAYER en escala de grises",
        "visual": "Caja cúbica monocroma con lazo claro para marcadores HUD",
    },
    {
        "path": "sprites/CanonPolo.png",
        "resource_line": 'SPRITE sprite_canon_polo "sprites/CanonPolo.png" 10 8 BEST 1 # Placeholder (Canon trineo fase polo principal)',
        "width": 80,
        "height": 64,
        "usage": "Cañón en trineo usado en la fase Polo Norte para disparar confeti",
        "frames": 2,
        "palette": "PAL_PLAYER con madera, rojo y metal gris",
        "visual": "Trineo rojo con cañón de madera, animación reposo/disparo",
    },
    {
        "path": "sprites/RedConfeti.png",
        "resource_line": 'SPRITE sprite_red_confeti "sprites/RedConfeti.png" 1 1 BEST 2 # Placeholder (Disparo red confeti polo rapido)',
        "width": 8,
        "height": 8,
        "usage": "Proyectil rápido de confeti en fase Polo Norte",
        "frames": 1,
        "palette": "PAL_EFFECT multicolor rojo y dorado",
        "visual": "Partícula compacta brillante ideal para ráfagas",
    },
    {
        "path": "sprites/CopoGrande.png",
        "resource_line": 'SPRITE sprite_copo_grande "sprites/CopoGrande.png" 8 8 BEST # Placeholder (Obstaculo copo nieve grande estatico)',
        "width": 64,
        "height": 64,
        "usage": "Obstáculo estático de copo de nieve grande en fase Polo Norte",
        "frames": 1,
        "palette": "PAL_COMMON blanco brillante",
        "visual": "Copo de nieve geométrico grande y legible",
    },
    {
        "path": "sprites/SantaCar.png",
        "resource_line": 'SPRITE sprite_santa_car "sprites/SantaCar.png" 10 16 BEST 2 # Placeholder (Carro vertical de Papa Noel para scroll)',
        "width": 80,
        "height": 128,
        "usage": "Carro de Papá Noel visto cenital con animación de nariz parpadeando",
        "frames": 2,
        "palette": "PAL_PLAYER rojos, verdes y dorados",
        "visual": "Rectángulo grande con trineo y reno, frame base y frame de nariz brillante",
    },
    {
        "path": "sprites/ArbolPista.png",
        "resource_line": 'SPRITE sprite_arbol_pista "sprites/ArbolPista.png" 8 8 BEST 1 # Placeholder (Arbol 64x64 decorativo/coleccionable)',
        "width": 64,
        "height": 64,
        "usage": "Árbol de pista de nieve que acompaña al scroll vertical",
        "frames": 1,
        "palette": "PAL_COMMON con verde y nieve",
        "visual": "Abeto cubierto de nieve en vista cenital",
    },
    {
        "path": "sprites/ElfoLateral.png",
        "resource_line": 'SPRITE sprite_elfo_lateral "sprites/ElfoLateral.png" 4 4 BEST 1 # Placeholder (Elfo con regalo en lateral)',
        "width": 32,
        "height": 32,
        "usage": "Elfo aliado en los laterales que lanza regalos",
        "frames": 1,
        "palette": "PAL_PLAYER con verdes y rojo navideño",
        "visual": "Elfo de perfil sosteniendo un regalo",
    },
    {
        "path": "sprites/DuendeMalo.png",
        "resource_line": 'SPRITE sprite_duende_malo "sprites/DuendeMalo.png" 4 4 BEST 1 # Placeholder (Duende enemigo 32x32)',
        "width": 32,
        "height": 32,
        "usage": "Duende malvado que se acerca lateralmente al carro",
        "frames": 1,
        "palette": "PAL_EFFECT con tonos morado y negro",
        "visual": "Criatura pequeña con gorro puntiagudo y mirada hostil",
    },
    {
        "path": "sprites/Regalos_Pequeno.png",
        "resource_line": 'SPRITE sprite_regalo_entrega "sprites/Regalos_Pequeno.png" 3 3 BEST 1 # Placeholder (Regalo pequeño entrega tejados volador)',
        "width": 24,
        "height": 24,
        "usage": "Regalo pequeño que se lanza a las chimeneas en fase de tejados",
        "frames": 1,
        "palette": "PAL_PLAYER con rojo y cinta dorada",
        "visual": "Caja cúbica miniatura con lazo superior, pensada para rotar en vuelo",
    },
    {
        "path": "sprites/Chimenea.png",
        "resource_line": 'SPRITE sprite_chimenea "sprites/Chimenea.png" 3 10 BEST 1 # Placeholder (Chimenea activable con regalo encendida)',
        "width": 20,
        "height": 160,
        "final_size": "20x80 píxeles por frame (2 frames apilados)",
        "usage": "Chimenea objetivo en fase tejados; cambia a encendida tras recibir regalo",
        "frames": 2,
        "palette": "PAL_PLAYER ladrillo gris/rojo con brillos de fuego",
        "visual": "Chimenea vertical estrecha con versión apagada y otra con humo y brasas",
    },
    {
        "path": "sprites/CanonTejado.png",
        "resource_line": 'SPRITE sprite_canon_tejado "sprites/CanonTejado.png" 8 12 BEST 1 # Placeholder (Canon tejado entrega regalos principal)',
        "width": 64,
        "height": 96,
        "usage": "Cañón móvil en tejados para disparar regalos hacia las chimeneas",
        "frames": 2,
        "palette": "PAL_PLAYER madera y rojo con destello de disparo",
        "visual": "Cañón sobre base de trineo con frame de retroceso y chispa",
    },
    {
        "path": "sprites/Nube.png",
        "resource_line": 'SPRITE sprite_nube "sprites/Nube.png" 8 4 BEST # Placeholder (Nube decorativa paralaje tejados suave)',
        "width": 64,
        "height": 32,
        "usage": "Nube de parallax lenta para fondo de tejados",
        "frames": 1,
        "palette": "PAL_COMMON blanco y gris suave",
        "visual": "Nube algodonosa simple para repetición en tilemap",
    },
    {
        "path": "sprites/Confeti.png",
        "resource_line": 'SPRITE sprite_confeti "sprites/Confeti.png" 1 1 BEST 2 # Placeholder (Particula confeti celebracion final colorido)',
        "width": 4,
        "height": 4,
        "usage": "Partícula de confeti en la fase de celebración",
        "frames": 1,
        "palette": "PAL_EFFECT multicolor brillante",
        "visual": "Pieza diminuta de papel de color sólido",
    },
    {
        "path": "sprites/PersonajeFiesta.png",
        "resource_line": 'SPRITE sprite_personaje "sprites/PersonajeFiesta.png" 8 8 BEST 1 # Placeholder (Personaje bailando fiesta final animado)',
        "width": 64,
        "height": 64,
        "usage": "Personaje celebrando y bailando en la fase final",
        "frames": 4,
        "palette": "PAL_PLAYER colores cálidos festivos",
        "visual": "Figura humana bailando con animación de 4 frames",
    },
    {
        "path": "sprites/ArbolFiesta.png",
        "resource_line": 'SPRITE sprite_arbol_fiesta "sprites/ArbolFiesta.png" 10 15 BEST 1 # Placeholder (Arbol navidad animado fiesta central)',
        "width": 80,
        "height": 240,
        "final_size": "80x120 píxeles por frame (2 frames verticales)",
        "usage": "Árbol de navidad protagonista en la sala de fiesta",
        "frames": 2,
        "palette": "PAL_PLAYER con verdes y luces multicolor",
        "visual": "Árbol alto con guirnaldas y dos estados de luces parpadeantes",
    },
    {
        "path": "sprites/Globo.png",
        "resource_line": 'SPRITE sprite_globo "sprites/Globo.png" 3 4 BEST 2 # Placeholder (Globo decoracion fiesta final colorido)',
        "width": 24,
        "height": 32,
        "usage": "Globos decorativos colgando en la fase de celebración",
        "frames": 1,
        "palette": "PAL_EFFECT rojo, azul y amarillo",
        "visual": "Globo ovalado con cuerda fina, 3 variantes de color",
    },
    {
        "path": "bg/FondoPolo.png",
        "resource_line": 'TILESET image_fondo_polo_tile "bg/FondoPolo.png" BEST # Placeholder (Fondo tileset fase polo principal)',
        "width": 512,
        "height": 256,
        "final_size": "320x224 píxeles visibles (tileset 512x256)",
        "usage": "Tileset base del fondo estático del Polo Norte sin parallax",
        "visual": "Patrón de nieve, árboles lejanos y luna en cielo frío",
    },
    {
        "path": "bg/FondoPolo.png",
        "resource_line": 'MAP image_fondo_polo_map "bg/FondoPolo.png" image_fondo_polo_tile BEST # Placeholder (Mapa fondo fase polo principal)',
        "width": 512,
        "height": 256,
        "final_size": "Mapa 40x28 tiles (320x224 en pantalla)",
        "usage": "MapDefinition del fondo principal del Polo Norte",
        "visual": "Distribución de nieve, árboles y horizonte polar",
    },
    {
        "path": "bg/PistaPolo.png",
        "resource_line": 'TILESET image_pista_polo_tile "bg/PistaPolo.png" BEST # Placeholder (Pista vertical fase uno)',
        "width": 320,
        "height": 448,
        "final_size": "Tileset 320x448 para scroll en bucle de dos pantallas",
        "usage": "Superficie de pista de nieve con vallas laterales",
        "visual": "Carril central claro y marcas de nieve en los bordes",
    },
    {
        "path": "bg/PistaPolo.png",
        "resource_line": 'MAP image_pista_polo_map "bg/PistaPolo.png" image_pista_polo_tile BEST # Placeholder (Mapa pista vertical fase uno)',
        "width": 320,
        "height": 448,
        "final_size": "Mapa 40x56 tiles para scroll vertical continuo",
        "usage": "Mapa principal de la pista polar cenital",
        "visual": "Trazado de nieve con vallas laterales repetibles",
    },
    {
        "path": "bg/CoposFondo.png",
        "resource_line": 'TILESET image_copos_fondo_tile "bg/CoposFondo.png" BEST # Placeholder (Capa copos paralaje polo fondo)',
        "width": 256,
        "height": 256,
        "final_size": "320x224 píxeles (tilemap de copos en parallax)",
        "usage": "Capa paralaje de copos y niebla en Polo Norte",
        "visual": "Copos pequeños blancos sobre fondo semi-transparente",
    },
    {
        "path": "bg/CoposFondo.png",
        "resource_line": 'MAP image_copos_fondo_map "bg/CoposFondo.png" image_copos_fondo_tile BEST # Placeholder (Mapa copos paralaje polo fondo)',
        "width": 256,
        "height": 256,
        "final_size": "Mapa 40x28 tiles para capa de copos",
        "usage": "MapDefinition del parallax de copos con scroll lento (-1 px/frame)",
        "visual": "Patrones distribuidos de nieve fina para paralaje",
    },
    {
        "path": "bg/FondoTejados.png",
        "resource_line": 'TILESET image_fondo_tejados_tile "bg/FondoTejados.png" BEST # Placeholder (Fondo base fase tejados principal)',
        "width": 512,
        "height": 256,
        "final_size": "320x224 píxeles (tileset 512x256)",
        "usage": "Tileset principal de la ciudad nocturna con tejados y chimeneas",
        "visual": "Tejados rojos/marrones, luna llena, estrellas y luces de casas",
    },
    {
        "path": "bg/FondoTejados.png",
        "resource_line": 'MAP image_fondo_tejados_map "bg/FondoTejados.png" image_fondo_tejados_tile BEST # Placeholder (Mapa fondo fase tejados principal)',
        "width": 512,
        "height": 256,
        "final_size": "Mapa 40x28 tiles (320x224 en pantalla)",
        "usage": "Mapa del fondo base de los tejados sin scroll",
        "visual": "Colocación de tejados, luna y luces distribuidas",
    },
    {
        "path": "bg/Nubes.png",
        "resource_line": 'TILESET image_nubes_paralaje_tile "bg/Nubes.png" BEST # Placeholder (Nubes paralaje fase tejados decorativas)',
        "width": 256,
        "height": 256,
        "final_size": "320x224 píxeles (parallax)",
        "usage": "Tileset de nubes para capa de paralaje en fase de tejados",
        "visual": "Nubes blancas suavemente sombreadas sobre fondo transparente",
    },
    {
        "path": "bg/Nubes.png",
        "resource_line": 'MAP image_nubes_paralaje_map "bg/Nubes.png" image_nubes_paralaje_tile BEST # Placeholder (Mapa nubes paralaje tejados decorativas)',
        "width": 256,
        "height": 256,
        "final_size": "Mapa 40x28 tiles para capa de nubes en scroll lento",
        "usage": "MapDefinition de nubes flotantes con desplazamiento -0.5 px/frame",
        "visual": "Patrones de nubes espaciadas para mosaico",
    },
    {
        "path": "bg/FondoFiesta.png",
        "resource_line": 'TILESET image_fondo_fiesta_tile "bg/FondoFiesta.png" BEST # Placeholder (Fondo sala fiesta final principal)',
        "width": 512,
        "height": 256,
        "final_size": "320x224 píxeles (tileset 512x256)",
        "usage": "Fondo base del salón de fiesta final",
        "visual": "Sala decorada con guirnaldas y luces cálidas",
    },
    {
        "path": "bg/FondoFiesta.png",
        "resource_line": 'MAP image_fondo_fiesta_map "bg/FondoFiesta.png" image_fondo_fiesta_tile BEST # Placeholder (Mapa fondo fiesta final principal)',
        "width": 512,
        "height": 256,
        "final_size": "Mapa 40x28 tiles (320x224 en pantalla)",
        "usage": "Mapa del fondo del salón de celebración",
        "visual": "Distribución de paredes, adornos y zona central",
    },
    {
        "path": "bg/LucesParpadeo.png",
        "resource_line": 'TILESET image_luces_parpadeo_tile "bg/LucesParpadeo.png" BEST # Placeholder (Capa luces parpadeo fiesta animada)',
        "width": 256,
        "height": 256,
        "final_size": "320x224 píxeles (overlay de luces)",
        "usage": "Capa de luces parpadeantes en la fiesta, anima cada 10 frames",
        "visual": "Motivos luminosos que se encienden/apagan de forma rítmica",
    },
    {
        "path": "bg/LucesParpadeo.png",
        "resource_line": 'MAP image_luces_parpadeo_map "bg/LucesParpadeo.png" image_luces_parpadeo_tile BEST # Placeholder (Mapa luces parpadeo fiesta animada)',
        "width": 256,
        "height": 256,
        "final_size": "Mapa 40x28 tiles (overlay de luces)",
        "usage": "MapDefinition de la capa animada de luces de fiesta",
        "visual": "Distribución de luces sobre el salón para parpadeo sincronizado",
    },
    {
        "path": "sfx/snd_regalo_recogido.wav",
        "resource_line": 'WAV snd_regalo_recogido "sfx/snd_regalo_recogido.wav" XGM2 # Placeholder (Sonido recoger regalo polo positivo)',
        "duration": 0.2,
        "usage": "Efecto positivo al recoger un regalo en el Polo Norte",
        "tone": "Ding metálico corto, limpio y alegre",
    },
    {
        "path": "sfx/snd_disparo_red.wav",
        "resource_line": 'WAV snd_disparo_red "sfx/snd_disparo_red.wav" XGM2 # Placeholder (Disparo red confeti polo rapido)',
        "duration": 0.1,
        "usage": "Sonido de disparo del confeti en fase Polo Norte",
        "tone": "Whoosh rápido y suave",
    },
    {
        "path": "sfx/snd_obstaculo_golpe.wav",
        "resource_line": 'WAV snd_obstaculo_golpe "sfx/snd_obstaculo_golpe.wav" XGM2 # Placeholder (Golpe contra copo obstaculo suave)',
        "duration": 0.15,
        "usage": "Impacto al golpear un copo de nieve obstáculo",
        "tone": "Thud de nieve amortiguado",
    },
    {
        "path": "sfx/snd_regalo_disparado.wav",
        "resource_line": 'WAV snd_regalo_disparado "sfx/snd_regalo_disparado.wav" XGM2 # Placeholder (Lanzamiento regalo desde canon tejados)',
        "duration": 0.15,
        "usage": "Lanzamiento de regalo desde el cañón en los tejados",
        "tone": "Whomp suave de disparo",
    },
    {
        "path": "sfx/snd_entrega_exitosa.wav",
        "resource_line": 'WAV snd_entrega_exitosa "sfx/snd_entrega_exitosa.wav" XGM2 # Placeholder (Regalo entregado en chimenea exitoso)',
        "duration": 0.3,
        "usage": "Confirmación al acertar una chimenea en fase de entrega",
        "tone": "Doble campanada brillante estilo ding-dong",
    },
    {
        "path": "sfx/snd_chimenea_activa.wav",
        "resource_line": 'WAV snd_chimenea_activa "sfx/snd_chimenea_activa.wav" XGM2 # Placeholder (Chimenea encendida tras regalo victoria)',
        "duration": 0.2,
        "usage": "Encendido de chimenea tras recibir regalo",
        "tone": "Pop de fuego con chispa",
    },
    {
        "path": "sfx/snd_confeti_choque.wav",
        "resource_line": 'WAV snd_confeti_choque "sfx/snd_confeti_choque.wav" XGM2 # Placeholder (Choque confeti celebracion final alegre)',
        "duration": 0.1,
        "usage": "Golpe suave de confeti en la celebración final",
        "tone": "Sonido pequeño y alegre de papel",
    },
    {
        "path": "music/musica_polo.wav",
        "resource_line": 'WAV musica_polo "music/musica_polo.wav" XGM2 # Placeholder (Musica ambiente fase polo inicial)',
        "duration": "30-60 segundos en loop",
        "usage": "Música ambiente de la fase Polo Norte (intro del juego)",
        "tone": "Navideña instrumental con piano y cuerdas a 120 BPM, estilo music box",
        "loop": True,
    },
    {
        "path": "music/musica_tejados.wav",
        "resource_line": 'WAV musica_tejados "music/musica_tejados.wav" XGM2 # Placeholder (Musica ambiente fase tejados nocturna)',
        "duration": "30-60 segundos en loop",
        "usage": "Música ambiente para la entrega nocturna en tejados",
        "tone": "Tema navideño medio tiempo con atmósfera nocturna",
        "loop": True,
    },
    {
        "path": "music/musica_celebracion.wav",
        "resource_line": 'WAV musica_celebracion "music/musica_celebracion.wav" XGM2 # Placeholder (Musica ambiente fase fiesta celebracion)',
        "duration": "30-60 segundos en loop",
        "usage": "Música festiva de la fase de celebración final",
        "tone": "Sintetizador alegre y rápido a 140 BPM, muy festivo",
        "loop": True,
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


def generate_beep(
    path: Path, duration: float = DEFAULT_AUDIO_DURATION, sample_rate: int = 22050
) -> None:
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


def extract_placeholder_detail(resource_line: str) -> str | None:
    if "#" not in resource_line:
        return None

    comment = resource_line.split("#", 1)[1].strip()
    marker = "placeholder"
    if not comment.lower().startswith(marker):
        return None

    detail = comment[len(marker) :].strip()
    detail = detail.strip("() ") or comment
    return detail


def describe_resource(entry: dict) -> str | None:
    detail = entry.get("usage") or extract_placeholder_detail(entry["resource_line"])
    if not detail:
        return None

    kind = entry["resource_line"].split()[0].upper()
    path = entry["path"]
    suffix = Path(path).suffix.lower()

    if suffix == ".wav":
        duration_value = entry.get("duration", DEFAULT_AUDIO_DURATION)
        if isinstance(duration_value, (int, float)):
            duration_text = f"{duration_value:.2f} segundos"
        else:
            duration_text = str(duration_value)

        tone = entry.get("tone")
        loop = entry.get("loop")
        extra_lines = []
        if tone:
            extra_lines.append(f" - Estilo/referencia sonora: {tone}")
        if loop:
            extra_lines.append(" - Debe permitir loop limpio sin clicks")

        prompt = [
            f"[{kind}] {path}",
            " - Tipo de recurso: audio en formato WAV (PCM 8kHz, mono)",
            f" - Aparición y uso en el juego: {detail}",
            f" - Duración prevista en el juego: {duration_text}",
        ]
        prompt.extend(extra_lines)
        prompt.append(
            " - Prompt IA: Diseña un efecto limpio sin ruido de fondo, listo para XGM2,"
            f" que encaje con la escena descrita y respete la duración objetivo ({duration_text})."
        )
        return "\n".join(prompt)

    width = entry.get("width", 64)
    height = entry.get("height", 64)
    final_size = entry.get("final_size") or f"{width}x{height} píxeles"
    frames = entry.get("frames")
    palette = entry.get("palette")
    visual = entry.get("visual")

    prompt_lines = [
        f"[{kind}] {path}",
        " - Tipo de recurso: imagen/sprite",
        f" - Aparición y uso en el juego: {detail}",
        f" - Tamaño final en el juego: {final_size}",
    ]

    if frames:
        prompt_lines.append(f" - Frames/animación: {frames} frames")
    if palette:
        prompt_lines.append(f" - Paleta/colores previstos: {palette}")
    if visual:
        prompt_lines.append(f" - Descripción visual: {visual}")

    prompt_lines.append(
        " - Prompt IA: Genera un gráfico nítido y legible a esa resolución,"
        f" que represente fielmente: {visual or detail}. Ajusta la composición para el"
        " contexto descrito, sin bordes ni márgenes vacíos."
    )
    return "\n".join(prompt_lines)


def print_placeholder_prompts(entries: list[dict]) -> None:
    for entry in entries:
        prompt = describe_resource(entry)
        if prompt:
            print(prompt)


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
    print_placeholder_prompts(ENTRIES)


if __name__ == "__main__":
    main()
