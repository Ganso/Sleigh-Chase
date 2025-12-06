PALETTE image_fondo_pal "bg/Fondo.png" BEST
PALETTE image_fondo_polo_pal "bg/FondoPolo.png" BEST # Placeholder (Paleta fondo polar fase uno)
PALETTE image_pista_polo_pal "bg/PistaPolo.png" BEST # Placeholder (Paleta pista scroll fase uno)
PALETTE image_fondo_tejados_pal "bg/FondoTejados.png" BEST # Placeholder (Paleta fondo tejados noche navidad)
PALETTE image_fondo_fiesta_pal "bg/FondoFiesta.png" BEST # Placeholder (Paleta fondo fiesta final colorido)

# Campanas y cañón
SPRITE sprite_campana "sprites/Campana.png" 4 4 BEST
SPRITE sprite_campana_bn "sprites/Campanabn.png" 4 4 BEST
SPRITE sprite_canon "sprites/Canon.png" 10 8 BEST 1
SPRITE sprite_bomba "sprites/Bomba.png" 4 4 BEST 1
SPRITE sprite_bola_confeti "sprites/BolaConfeti.png" 1 1 BEST 1

# Fase 1
SPRITE sprite_regalo "sprites/Regalo.png" 4 4 BEST 1 # Placeholder (Regalo cayendo en fase polar)
SPRITE sprite_regalo_bn "sprites/RegaloBN.png" 4 4 BEST 1 # Placeholder (Contador regalo blanco y negro)
SPRITE sprite_canon_polo "sprites/CanonPolo.png" 10 8 BEST 1 # Placeholder (Canon trineo fase polo principal)
SPRITE sprite_red_confeti "sprites/RedConfeti.png" 1 1 BEST 2 # Placeholder (Disparo red confeti polo rapido)
SPRITE sprite_copo_grande "sprites/CopoGrande.png" 8 8 BEST # Placeholder (Obstaculo copo nieve grande estatico)
SPRITE sprite_santa_car "sprites/SantaCar.png" 10 16 BEST 2 # Placeholder (Carro vertical de Papa Noel para scroll)
SPRITE sprite_arbol_pista "sprites/ArbolPista.png" 8 8 BEST 1 # Placeholder (Arbol 64x64 decorativo/coleccionable)
SPRITE sprite_elfo_lateral "sprites/ElfoLateral.png" 4 4 BEST 1 # Placeholder (Elfo con regalo en lateral)
SPRITE sprite_duende_malo "sprites/DuendeMalo.png" 4 4 BEST 1 # Placeholder (Duende enemigo 32x32)

# Fase 2
SPRITE sprite_regalo_entrega "sprites/Regalos_Pequeno.png" 3 3 BEST 1 # Placeholder (Regalo pequeño entrega tejados volador)
SPRITE sprite_chimenea "sprites/Chimenea.png" 3 10 BEST 1 # Placeholder (Chimenea activable con regalo encendida)
SPRITE sprite_canon_tejado "sprites/CanonTejado.png" 8 12 BEST 1 # Placeholder (Canon tejado entrega regalos principal)
SPRITE sprite_nube "sprites/Nube.png" 8 4 BEST # Placeholder (Nube decorativa paralaje tejados suave)

# Fase 4
SPRITE sprite_confeti "sprites/Confeti.png" 1 1 BEST 2 # Placeholder (Particula confeti celebracion final colorido)
SPRITE sprite_personaje "sprites/PersonajeFiesta.png" 8 8 BEST 1 # Placeholder (Personaje bailando fiesta final animado)
SPRITE sprite_arbol_fiesta "sprites/ArbolFiesta.png" 10 15 BEST 1 # Placeholder (Arbol navidad animado fiesta central)
SPRITE sprite_globo "sprites/Globo.png" 3 4 BEST 2 # Placeholder (Globo decoracion fiesta final colorido)

# Fondo
TILESET image_fondo_tile "bg/Fondo.png" BEST
MAP image_fondo_map "bg/Fondo.png" image_fondo_tile BEST
TILESET image_fondo_nieve_tile "bg/FondoNieve.png" BEST
MAP image_fondo_nieve_map "bg/FondoNieve.png" image_fondo_nieve_tile BEST
TILESET image_fondo_polo_tile "bg/FondoPolo.png" BEST # Placeholder (Fondo tileset fase polo principal)
MAP image_fondo_polo_map "bg/FondoPolo.png" image_fondo_polo_tile BEST # Placeholder (Mapa fondo fase polo principal)
TILESET image_pista_polo_tile "bg/PistaPolo.png" BEST # Placeholder (Pista vertical fase uno)
MAP image_pista_polo_map "bg/PistaPolo.png" image_pista_polo_tile BEST # Placeholder (Mapa pista vertical fase uno)
TILESET image_copos_fondo_tile "bg/CoposFondo.png" BEST # Placeholder (Capa copos paralaje polo fondo)
MAP image_copos_fondo_map "bg/CoposFondo.png" image_copos_fondo_tile BEST # Placeholder (Mapa copos paralaje polo fondo)
TILESET image_fondo_tejados_tile "bg/FondoTejados.png" BEST # Placeholder (Fondo base fase tejados principal)
MAP image_fondo_tejados_map "bg/FondoTejados.png" image_fondo_tejados_tile BEST # Placeholder (Mapa fondo fase tejados principal)
TILESET image_nubes_paralaje_tile "bg/Nubes.png" BEST # Placeholder (Nubes paralaje fase tejados decorativas)
MAP image_nubes_paralaje_map "bg/Nubes.png" image_nubes_paralaje_tile BEST # Placeholder (Mapa nubes paralaje tejados decorativas)
TILESET image_fondo_fiesta_tile "bg/FondoFiesta.png" BEST # Placeholder (Fondo sala fiesta final principal)
MAP image_fondo_fiesta_map "bg/FondoFiesta.png" image_fondo_fiesta_tile BEST # Placeholder (Mapa fondo fiesta final principal)
TILESET image_luces_parpadeo_tile "bg/LucesParpadeo.png" BEST # Placeholder (Capa luces parpadeo fiesta animada)
MAP image_luces_parpadeo_map "bg/LucesParpadeo.png" image_luces_parpadeo_tile BEST # Placeholder (Mapa luces parpadeo fiesta animada)

# Sonidos
WAV snd_campana "sfx/snd_campana.wav" XGM2
WAV snd_bomba "sfx/snd_bomba.wav" XGM2
WAV snd_canon "sfx/snd_canon.wav" XGM2
WAV snd_regalo_recogido "sfx/snd_regalo_recogido.wav" XGM2 # Placeholder (Sonido recoger regalo polo positivo)
WAV snd_disparo_red "sfx/snd_disparo_red.wav" XGM2 # Placeholder (Disparo red confeti polo rapido)
WAV snd_obstaculo_golpe "sfx/snd_obstaculo_golpe.wav" XGM2 # Placeholder (Golpe contra copo obstaculo suave)
WAV snd_regalo_disparado "sfx/snd_regalo_disparado.wav" XGM2 # Placeholder (Lanzamiento regalo desde canon tejados)
WAV snd_entrega_exitosa "sfx/snd_entrega_exitosa.wav" XGM2 # Placeholder (Regalo entregado en chimenea exitoso)
WAV snd_chimenea_activa "sfx/snd_chimenea_activa.wav" XGM2 # Placeholder (Chimenea encendida tras regalo victoria)
WAV snd_confeti_choque "sfx/snd_confeti_choque.wav" XGM2 # Placeholder (Choque confeti celebracion final alegre)

# Música
XGM2 musica_fondo "music/musica.vgm"
WAV musica_polo "music/musica_polo.wav" XGM2 # Placeholder (Musica ambiente fase polo inicial)
WAV musica_tejados "music/musica_tejados.wav" XGM2 # Placeholder (Musica ambiente fase tejados nocturna)
WAV musica_celebracion "music/musica_celebracion.wav" XGM2 # Placeholder (Musica ambiente fase fiesta celebracion)
