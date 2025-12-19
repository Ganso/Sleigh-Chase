# Borrador de post: como cree *Sleigh Chase* (guia rapida de desarrollo Mega Drive acelerado con IA)

> **Nota personal**: esta es una version borrador, pensada para sonar cercana y amable, sin entrar en tecnicismos en exceso. Sientete libre de retocar, recortar o ampliar.

## Introduccion

Si alguna vez has querido crear un juego para Sega Mega Drive/Genesis pero te ha faltado tiempo, equipo o confianza tecnica, este post es para ti. Voy a contar, de forma directa y sencilla, como he creado **Sleigh Chase**, un minijuego navideno para Mega Drive, usando herramientas de IA como Codex, Eleven Labs y Nano Banana. Mi idea es ofrecer una guia rapida y realista, pensada para personas que no tienen un conocimiento profundo de programacion, pero si curiosidad y ganas.

El objetivo no es reemplazar el trabajo humano, sino **abrir puertas**. Para mi, usar IA en este contexto se parece mucho a usar un motor de videojuegos: simplifica tareas complejas y permite que mas gente pueda crear, sin necesidad de un equipo grande ni anos de formacion.

Tambien creo que trabajar con IA se parece mucho al papel de **director de proyecto**, y en SCRUM encaja bastante con el rol de **Product Owner**: defines que quieres, lo partes en tareas pequenas, priorizas el backlog y haces **iteraciones cortas** que te lleven a resultados concretos y reversibles. En lugar de pedir un bloque enorme de codigo, es mejor avanzar paso a paso, validar, y si algo falla, retroceder sin dolor.

---

## Prerrequisitos (lo minimo que conviene asumir)

Antes de meternos en IA, hay una base real que conviene tener clara. No es para asustar, es para evitar frustraciones:

1. **Usar GitHub como base del proyecto.** El flujo con Codex (web) y validacion de cambios se apoya en GitHub, asi que parto de que el repositorio vive ahi.
2. **Conocer lo fundamental del hardware Mega Drive/Genesis.** Esta consola tiene muchas particularidades, sobre todo con la **VRAM y las paletas**. Cosas clave:
   - **Tiles de 8x8 y planos (BG A/B)**: todo el render en 2D se basa en tiles, y los fondos se componen en planos con mapas de tiles.
   - **VRAM limitada**: hay que planificar que tiles entran y salen. Reusar tiles es obligatorio si quieres escenas ricas.
   - **Paletas limitadas**: 4 paletas de 16 colores (con transparencia). Si un sprite o fondo necesita mas colores, hay que simplificar o dividir bien por paletas.
   - **Sprites y limites**: hay maximo de sprites por linea y por frame, asi que hay que controlar cuantos se dibujan.
   - **DMA y VBlank**: muchas cargas de tiles o paletas se hacen en VBlank y conviene ordenar las cargas con cuidado.
3. **Saber pixelar y animar.** Yo genero graficos siempre a tamano grande y **sin pedir pixelado**. Luego adapto con **Aseprite**, que recomiendo totalmente. La animacion es la parte **menos automatizable**: los frames tienen que funcionar bien y ser coherentes. Para que los fondos entren en VRAM, muchas veces hay que **simplificarlos** y forzar que usen tiles reutilizables.
4. **Audio sin complicaciones**: no hace falta modificarlo mucho a mano, porque **rescomp** se encarga de bajar la calidad al formato que usa el hardware.
5. **Musica**: de momento hay pocas ayudas reales con IA para secuenciar musica en Mega Drive. Aqui tuve la suerte de contar con una persona real, **Haddhar**, que hizo todo el trabajo en DefleMask.

Y una recomendacion muy practica: **lo ideal es comenzar importando un proyecto que sirva como plantilla**. En mi caso use la felicitacion navidena que hice el ano pasado: https://github.com/Ganso/Feliz2025, pero el juego de 2026 (mucho mas completo) puede servir tambien como base. En el GitHub de SGDK se enlaza a muchos proyectos que ofrecen una buena base, y el propio SDK trae ejemplos con codigo que podemos pasarle como referencia a Codex para que aprenda el estilo correcto. Las IAs saben mucho de C y de desarrollo desde un punto de vista academico, pero no tienen mucha experiencia en Mega Drive y otros sistemas retro, asi que es fundamental darles todo el contexto posible.

---

## Disclaimer etico (importante)

El uso de IA generativa tiene implicaciones eticas reales: desde el impacto en empleos creativos, hasta el origen de los datos con los que se entrenan los modelos. Soy consciente de ello, y creo que lo correcto es **ser transparente** y **compartir el resultado** para aportar valor a la comunidad. En mi caso, todo el codigo fuente, recursos y ficheros originales estan abiertos en GitHub. Esta es mi manera de devolver algo a cambio.

Y repito la idea central: **usar IA no es hacer trampa**, es una herramienta mas. Igual que un motor como Unity o Godot ayuda a quien no tiene equipo o tiempo, la IA puede ayudar a prototipar, aprender y crear.

---

## El juego: Sleigh Chase (resumen)

Sleigh Chase es un minijuego navideno para Sega Mega Drive/Genesis, con la intencion de ser divertido y facil de disfrutar en familia en estas fiestas. Es nuestra manera de felicitar la navidad y el nuevo ano 2026.

Consta de tres minijuegos:

1. **Recogida de regalos**: Los malvados secuaces del Grinch han robado los ultimos 10 regalos que le quedan por repartir a Papa Noel. Los elfos verdes nos los lanzan desde los laterales, y tenemos que recogerlos esquivando los arboles y a los duendes rojos, que nos robaran los regalos que hayamos recogido.
2. **Entrega en la ciudad**: Sobrevolamos la ciudad con nuestro trineo, buscando diez chimeneas a las que lanzarle los regalos (boton A del mando). No los lances a chimeneas encendidas, y cuidado de nuevo con los duendes malignos que trataran de interceptarlos.
3. **Campanadas**: Despues de salvar la navidad, tocaremos las doce campanas lanzandoles confeti, y formaremos el mensaje de felicitacion de ano nuevo. No toques las bombas, o tendras que volver a empezar.

Todo el codigo del proyecto se ha creado integramente con Codex de OpenAI utilizando la libreria SGDK, tanto en su version web como integrado en VS Code, tomando como base el minijuego de felicitacion del ano pasado, que solo incluia una fase. Los recursos graficos han sido disenados con Nano Banana y adaptados con Aseprite, mientras que los efectos de sonido proceden de generacion con Eleven Labs.

Todas las musicas han sido secuenciadas desde cero por Haddhar en DefleMask.

---

## Area principal: generacion de codigo con Codex

La parte mas fuerte de todo el proceso, con diferencia, ha sido **la generacion del codigo**. Aqui es donde Codex se ha convertido en mi "copiloto" principal, tanto desde la web como desde VS Code.

### Puntos fuertes (mi experiencia real)

- **Iteracion rapida**: puedo pedir cambios muy concretos y obtener una propuesta viable en minutos.
- **Acceso remoto**: estando fuera de casa, puedo pedirle cosas a Codex (incluso desde el movil), revisar propuestas y validarlas, y luego ya lo tengo todo preparado para compilar en casa.
- **Modo local en VS Code**: cuando estoy en el escritorio, usar Codex integrado en el editor me permite revisar cambios rapido y entender el contexto del proyecto.

Aqui voy a pegar un screenshot de un uso que hice desde la sala de espera del medico.

> [AQUI VA LA CAPTURA DE PANTALLA]

### Codex via web: flujo con PRs en GitHub

Desde la web, el flujo suele ser:

1. Describir el cambio con detalle.
2. Codex analiza el repo y propone modificaciones.
3. Se generan cambios con contexto y diffs claros.
4. Se valida y se crea la solicitud de extraccion (PR) automaticamente.
5. Reviso el PR en GitHub y, si todo esta bien, lo acepto.

Lo interesante es que **puedes revisar y aprobar desde cualquier sitio** y luego, cuando vuelves a casa, solo compilas y pruebas el resultado. Es una forma de trabajo super practica.

Importante: **la compilacion siempre se hace en local con SGDK** (en mi caso, con VS Code), porque Codex no tiene las herramientas para compilar por si mismo. Al usar GitHub y herramientas web, este flujo encaja genial si dejas el entorno de desarrollo preparado para usarlo desde cualquier ordenador (casa, oficina, portatil, etc.).

### Codex en VS Code: flujo local

En VS Code, Codex trabaja directamente con los archivos locales:

- Le indicas la tarea (como si fuese un encargo tecnico).
- El modelo edita el codigo donde toca y te muestra los cambios.
- Tu revisas linea por linea, ajustas detalles y confirmas.

Este flujo es ideal para tareas mas pequenas o cuando quieres mantener un control milimetrico.

### Ver todo el repositorio (y alternativas de plugins)

Algo clave: **es muy relevante usar un sistema que vea el repositorio completo**, no solo el fichero que le pasas. En Mega Drive hay muchas referencias cruzadas (assets, headers, orden de cargas), y sin ese contexto la IA se pierde o propone cambios peligrosos.

Otra posibilidad es utilizar plugins de IA para VS Code. Funcionan bien, pero el gasto en tokens suele ser **muchisimo mas caro** que una cuota mensual de OpenAI, que en mi caso ha sido mas que suficiente para sacar adelante un proyecto como este. Si aun asi quieres ir por esa via, tienes alternativas como **Cline** o **Roo code**, y puedes usar **Openrouter** como intermediario para acceder a multiples modelos: desde algunos muy potentes pero mas caros como **Claude** para tareas complejas, hasta opciones gratuitas como **Deepseek** para tareas mas sencillas que requieran menos razonamiento.

---

## Dos ejemplos reales (copiados literalmente)

Aqui van dos solicitudes reales que le hice a Codex para este juego. Las copio tal cual:

### Ejemplo 1

```
En la pantalla de título, vamos a poner un selector de inglés o español
Primero, carga el font en modo dark, igual que lo hace la librería de cutscenes
Tras el scroll de Sleigh Chase, en una línea que esté aproximadamente al 60% de la pantalla pon "} ENGLISH {" centrado, y dos líneas más abajo "ESPA^OL" también centrado
Si pulsamos abajo o arriba, cambian: "ENGLISH" (sin llaves) y "} ESPA^OL {" (con llaves)
Al pulsar cualquier botón para empezar, se guardará en una variable global el idioma seleccionado, que será el que tenga las llaves puestas
```

### Ejemplo 2

```
Cuando el protagonista choca con un árbol, se pausa la música, suena un sonido de choque (actualmente tenemos un placeholder), se hace un parpadeo del sprite del personaje y el del árbol durante 1 segundo (más o menos 1 parpadeo cada 0.1s), y luego desaparece el árbol, el personaje vuelve a la posición inicial y se reanuda el juego y la música (con volumen a 0, y luego XGM2_fadeIn hasta el volumen estándar). Mantén la mecánica de que se pierde un regalo si lo tienes.
```

---

## Como trabaja Codex por dentro (desde la perspectiva del usuario)

Sin entrar en temas demasiado tecnicos, lo que he observado es lo siguiente:

- Codex **lee el repo**, identifica los archivos relevantes y localiza donde estan las funciones, estructuras o datos que necesita tocar.
- Si hace falta, **crea scripts auxiliares** (por ejemplo en Python) para inspeccionar contenido o verificar datos. Eso aparece en los registros y te permite entender por que toma ciertas decisiones.
- Luego propone cambios y los explica. Tu decides si aceptar o no. En ese sentido, no es una "caja negra" total: puedes seguir el rastro si quieres.

En resumen, es como trabajar con un asistente que sabe buscar rapido, organizar la informacion y escribir borradores que luego tu terminas de validar.

En mi caso, para este proyecto **yo no programe casi nada**: le pedi a la IA que fuese profusa en comentarios (usando formato **Doxygen**) y que todo quedase **parametrizable** tocando constantes. Asi puedo ajustar la jugabilidad sin tener que volver a la IA para cada detalle pequeno.

---

## Audio con Eleven Labs (samples rapidos y utiles)

Para los efectos de sonido, he usado **Eleven Labs**. La web es directa: eliges un estilo, generas variaciones y descargas el sample. Luego lo adaptas al formato necesario para SGDK. Para prototipos es una maravilla, porque **no necesitas grabar ni editar desde cero**.

---

## Graficos con Nano Banana + Aseprite

Los graficos los he generado con **Nano Banana**, y luego los he ajustado en Aseprite para el estilo Mega Drive. La combinacion funciona muy bien: IA para generar ideas, y Aseprite para limpiar, recolorear y adaptar a las limitaciones reales de la consola.

---

## Suscripciones y herramientas (lo que me ha funcionado)

- **OpenAI (Codex)**: con una suscripcion de un mes puedes acceder a Codex. Eso si, puedes gastar los tokens semanales y tener que esperar al refresco.
- **Perplexity**: un complemento muy interesante. Regala un ano de cuenta Pro al tener distintos servicios vinculados. En concreto, **hasta final de 2025 se puede reclamar si tienes una cuenta de PayPal creada desde hace tiempo**.

No necesitas gastar una fortuna: con una combinacion minima, ya tienes una base potente para crear.

---

## Consejos practicos para trabajar con Codex

### 1) Guarda referencias dentro del repo

Es muy util tener ficheros de referencia en las carpetas de GitHub. En mi caso tengo:

- Un fichero con la documentacion completa de SGDK compilada en uno solo (`documentos/sgdk-reference-2025-11-15.txt`).
- Otros documentos de diseno y especificacion.
- Y especialmente **`documentos/AYUDA_IA.md`**, que contiene una guia rapida con normas concretas del proyecto.

En ese fichero `AYUDA_IA.md` dejo cosas como:

- No compilar desde la IA.
- Mantener `README.md` actualizado si hay cambios funcionales.
- Evitar tocar ficheros generados por SGDK.
- Recordatorios sobre arquitectura, fases y manejo de recursos.

Esto ayuda muchisimo a que Codex responda con cambios coherentes y seguros.

### 2) Usa instrucciones personalizadas

Otro truco clave es fijar instrucciones personalizadas para Codex. Yo uso literalmente estas (copio y pego):

> "Para juegos de Megadrive no trates nunca de compilar el resultado, ya que debe hacerse fuera de tu entorno. Consulta la documentación de SGDK para ver la sintaxis de las funciones que puedes utilizar"

Parece simple, pero marca la diferencia.

---

## Cierre: por que merece la pena

Crear un juego retro suena grande, pero con las herramientas adecuadas y un enfoque pragmatico, es totalmente posible. Codex me ha permitido trabajar mas rapido, experimentar sin miedo y, sobre todo, **disfrutar del proceso**.

Si estas leyendo esto y te pica la curiosidad, mi consejo es claro: empieza pequeno, apóyate en IA como si fuese tu equipo tecnico, y poco a poco ve dandole forma. No necesitas ser un experto para crear algo bonito.

Y si quieres curiosear todo el material, el codigo fuente y los recursos de Sleigh Chase estan abiertos. Me encantaria que este proyecto te sirva como punto de partida o inspiracion.

---

## Extra: lista corta de pasos recomendados (resumen)

1. Define una idea simple y acotada.
2. Prepara un repo con referencias (docs, ayudas, ejemplos).
3. Usa Codex para tareas grandes y VS Code para rematar.
4. Genera audio y graficos con IA, y ajusta con herramientas clasicas.
5. Prueba, ajusta, repite.

Y sobre todo: **diviertete creando**.
