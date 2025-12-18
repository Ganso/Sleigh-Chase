# Presentación para Itch.io

Feliz 2026 es un homenaje navideño pensado para **Sega Mega Drive/Genesis**, con estética retro y controles rápidos tipo arcade. Siguiendo la tradición de los cartuchos festivos, el juego mantiene la sencillez visual de 16 bits mientras pone el foco en la rejugabilidad y en las rachas cortas de puntuación.

La aventura se divide en tres fases enlazadas. En **Recogida**, Santa sobrevuela el Polo Norte en scroll vertical con inercia total, esquivando árboles y enemigos mientras recoge los regalos que marcarán el resto del recorrido. En **Entrega**, el trineo se posa sobre tejados nevados y un cañón lateral lanza paquetes a chimeneas que se activan por oleadas, poniendo a prueba la puntería y la gestión del viento. En **Campanadas**, el ritmo se acelera: hay que encender campanas móviles para desbloquear la palabra final FELIZ2026 disparando letras en orden y esquivando bombas.

Todo el código del proyecto se ha creado íntegramente con **Codex de OpenAI**, tanto en su versión web como integrado en VS Code, tomando como base el minijuego de felicitación de 2025 que solo incluía una fase. Los recursos gráficos han sido diseñados con **Nano Banana** y adaptados con **Aseprite**, mientras que los efectos de sonido proceden de generación con **Eleven Labs**. La música es la única parte totalmente artesanal: compuesta y secuenciada por **Haddhar** en **DefleMask** para sonar nativa en el chipset YM2612/PSG.

El juego está optimizado para ejecutarse en hardware real o emuladores de Mega Drive, reutilizando la librería **SGDK** para gestionar sprites, mapas y audio XGM. No es necesaria instalación extra en Itch.io: el cartucho se distribuye como ROM lista para ser grabada o emulada, manteniendo siempre los 60 FPS.
