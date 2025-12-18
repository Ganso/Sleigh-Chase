# Sleigh Chase (Mega Drive) by Geesebumps

Juego de minijuegos navidenos para Sega Genesis/Mega Drive desarrollado con SGDK.

## Fases

- **Recogida de regalos**: Los malvados secuaces del Grinch han robado los últimos 10 regalos que le quedan por repartir a Papá Noel. Los elfos verdes, nos los lanzarán desde los laterales, y tenemos que recogerlos esquivando los árboles y a los duendes rojos, que nos robarán los regalos que hayamos recogido.
- **Entrega en la ciudad**: Sobrevolamos la ciudad con nuestro trineo, buscando diez chimeneas a las que lanzarle los regalos (botón A del mando). No los lances a chimeneas encendidas, y cuidado de nuevo con los duendes malignos que tratarán de interceptarlos.
- **Campanadas**: Después de salvar la navidad, tocaremos las doce campanas lanzándoles confeti, y formaremos el mensaje de felicitación de año nuevo. No toques las bombas, o tendrás que volver a empezar.

## Librerías

- SGDK (Sega Genesis Development Kit)

## Estructura
- `src/`: codigo C de fases y sistemas (core, audio, HUD, nieve, intro).
- `inc/`: headers correspondientes.
- `res/`: definiciones `.res` y recursos generados (`resources_*.h`, `res_geesebumps.h`).
- `documentos/`: documentacion del proyecto y referencia SGDK (`documentos/sgdk-reference-2025-11-15.txt`).

## Notas de desarrollo

- **Uso de IA**: Todo el código del proyecto se ha creado íntegramente con Codex de OpenAI utilizando la librería SGDK, tanto en su versión web como integrado en VS Code, tomando como base el minijuego de felicitación del año pasado, que solo incluía una fase. Los recursos gráficos han sido diseñados con Nano Banana y adaptados con Aseprite, mientras que los efectos de sonido proceden de generación con Eleven Labs.

- **Música**: Todas las músicas han sido secuenciadas desde cero por Haddhar en DefleMask.

- **Licencia**: Todo el código fuente, los recursos, y los ficheros originales se ofrecen de manera íntegra, totalmente abierta y sin ninguna restricción de uso. Somos conscientes de las implicaciones éticas del uso de IA generativa, y es nuestra manera de tratar de apoyar a la comunidad de desarrolladores ofreciendo el juego como plantilla para cualquiera que quiera desarrollar su proyecto.