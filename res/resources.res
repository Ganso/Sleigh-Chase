PALETTE image_fondo_pal "bg/Fondo.png" BEST

# Campanas y cañón
SPRITE sprite_campana "sprites/Campana.png" 4 4 BEST 
SPRITE sprite_campana_bn "sprites/Campanabn.png" 4 4 BEST 
SPRITE sprite_canon "sprites/Canon.png" 10 8 BEST 1
SPRITE sprite_bomba "sprites/Bomba.png" 4 4 BEST 1
SPRITE sprite_bola_confeti "sprites/BolaConfeti.png" 1 1 BEST 1

# Fondo
TILESET image_fondo_tile "bg/Fondo.png" BEST
MAP image_fondo_map "bg/Fondo.png" image_fondo_tile BEST 
TILESET image_fondo_nieve_tile "bg/FondoNieve.png" BEST
MAP image_fondo_nieve_map "bg/FondoNieve.png" image_fondo_nieve_tile BEST 

# Sonidos
WAV snd_campana "sfx/snd_campana.wav" XGM2
WAV snd_bomba "sfx/snd_bomba.wav" XGM2 
WAV snd_canon "sfx/snd_canon.wav" XGM2 

# Música
XGM2 musica_fondo "music/musica.vgm"
