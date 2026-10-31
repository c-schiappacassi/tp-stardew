# 1er Reunion 13-10-2026
### Pre-reunion

| Rol   | Alumno    | Función   |
| :---  | :---      | :---      |
|A      | Ciro      |implementa la parte de SDL del cliente|
|B      | Luisana   |implementa la parte de QT del cliente (con editor)|
|C      | Caro      |implementa la parte de comunicación entre cliente y servidor, threads, protocolo, testing y tareas extras (persistencia, configuración, etc)|
|D      | Agus      |implementa la parte de lógica del servidor|

### Preguntas
- "Para jugar con amigos, cada partida contará con un id único de invitación, que se le enviará a los demás jugadores que desean participar"
    
    --> ese id es el id de partida? es un token? si es token se regenera en el momento o siempre es el mismo?

    abril dice:
    "criterio propio, no se la compliquen, que cumpla el enunciado y listo. token es mucho quilombo"

- La partida multiplayer no tiene limite de jugadores entonces?
    
    --> cuatro como en ele juego real?

    abril dice:
    "si, como la del juego real"
    asumimos que el maximo historico que se puede unir a una partida es 4 (como en el juego)

- Cuando el jugador llega a 0% de vida y cae inconsciente:
    
    --> despertas en tu casa, con menos vida? o es como que se da el OK para el siguiente día(single o multiplayer)?

    abril dice:
    "pierde el resto del dia, es como irse a dormir y se levanta con menos vida y energia. en caso multiplayer es lo mismo, espera al resto para poder ir al dia siguiente"

- Cuanto oro se droppea al morir?
    
    abril dice:
    "cantidad a nuestro criterio, va a los configurables toml"

- "Robin: Amigable y talentosa, Robin crea en su taller productos de decoración, incluyendo cofres, que los vende al jugador para su granja y casa. Su catálogo cambia cada día, alternando de a 7 productos por día"
    
    --> cuales son TODOS esos productos de los que elegimos 7 por dia?

    abril dice:
    "cofre no rota, despues la deco que quieran, la que este en los assets o no, valores a los config toml"

- lo de los sets completados se ve como en una pagina de "misiones" o asi y van apareciendo con ticks? (como en clash of clans las "misiones" que te dan gemas?)
    
    --> es igual al juego?

    abril dice:
    "falta el interno de los assets de el centro comunitario. ver como queremos"

- Las minas tienen 10 layouts, las diseñamos nosotros?

    abril dice:
    "se crean en el editor. edita todo, pueblo, granja y minas. Varia los enemigos en cada visita"

- NO ENTIENDO EL EDITOR NADA QUE VER CON EL JUEGO

    abril dice:
    "es mario maker, edita y crea mundo. tipos de terreno, deco, todo. terraforming incluido. check de mundo valido"

- Eventos viven del lado cliente, el servidor no sabe nada de eventos no? solo conoce request

    abril dice:
    "exacto, no valida ni si es valido lo pedido"

- Esta bien una Queue para request de todas las partidas? o tener request Queue para cada partida?
 
    --> seria un thread por partida? en ese caso debe existir un thread más que maneja los clientes que todavia no esten en partida, que los deriva a queue de la partida correspondiente?

    abril dice:
    "cada partida es un hilo, tiene su propio gameloop con una "
    "QT es para menu y para editor: no hilos"
    "SDL libreria para el juego en si"
    "QT Y SDL VAN AL HILO MAIN"
    "QT NO SE CONTACTA CON EL SERVIDOR, armarmos todo empezar partida y el acceptor se crea con la data toda junta. mensaje handshake"

- No esta el hacha como herramienta inicial, deberiamos ponerlo en la tienda de Robin?

    abril dice:
    "tiene que estar"

- Linea de tiempo de entregables mencionada en las recomendaciones?

    abril dice:
    "nos lo manda, igual en cada reunion nos dice que se espera para la proxima"

- Chat unico para usuarios, comunicación con npcs y cheats

### Recomendaciones
- no merguear la rama hasta que el feature este completo (servidor y cliente - full circle)
- HAGANLO LINDO - LO ESTÉTICO VENDE - QUEREMOS PRETTY PRIVILEGE
- seguir lso consejos de manejo de ramas en git
- No uso irresponsable de ia obvio ( usenla para sintax en todo caso )


### TODO para la proxima reunión:
- features: 
    - musica y sonidos(efectos y musica de fondo)
    - cheats (soporte de protocolo) (/cheat )
- diseño de implementación  (reunion mínimo C y D)
    - cant hilos, clases, modelado de datos, seguir KISS, composición por sobre la herencia, etc...
- conexión cliente-servidor simple funcionando para tener de base. 
- SDL y QT: investiguen en la semana (QT juga con inputs) (SDL pp haga algo y se pueda interactuar)


# 2da Reunión 20-10-2026

### Preguntas y respuestas
- **El server solo envia struct serverStatus a los clientes**
- **Cada vez qeu corres el juego, el cliente tiene que ingresar su nombre de usuario(unico e identificador de cliente)**
- Map de funciones: se guarda una referencia a la funcion, el map te devuelve eso y lo ejecutas
- Class Player: no compartido
- Se puede usar std::variant? abril dice que se puede usar eso o herencia. como quieran
- Enums para los items o toml puro? enums
- Hitbox: servidor, sdl manda al servidor la posicion a donde voy, el server me devuelve donde termino estando (si se pudo, me va a devolver la pos que le mande, si no, me devuelve donde estaba)
- Dimenciones de los colisionables en un constastes.h
- Cliente: antes de renderizar chequear que del serverStatus entra en mi vista/cuadrado 
- Estaciones?  una vez que este lo del enuciado podemos ver si llegamos con extras

### TODO para la proxima reunión:
- QT: para pantallas de inicio son 2 semanas (ingresar ususario, crear partida, elegir mapa, elegir aspecto, joinear partida)
- Protocolo:
    - una semana: base del protocolo (ej hacer el serializador, definir que info se manda y su modelado)
    - 2 semanas: concurrencia(poder tener multiples partidas y multiples juagdores)
- Servidor + SDL:
    - Constant Rate loop - gameloop - leer el articulo
    - feat/movimiento completo
    - dto directo cada comando del sdl
- feat/mineria + inventario (1 semana)
  - picar los minerales
  - inventario (10 slots de stacks de 50)
  - conseguir la reconpensa de minar
- feat/granja (3 semanas)
  - logica animales (caminan)

# 3ra Reunión 27-10-2026
- preguntas: Zonas separadas por una loading screen como en el juego? - como les resulte más simple

### Para la semana que viene:

#### lo que no llegamos
- QT: para pantallas de inicio son 2 semanas (ingresar ususario, crear partida, elegir mapa, elegir aspecto, joinear partida)
- Protocolo:
    - 2 semanas: concurrencia(poder tener multiples partidas y multiples juagdores)
- Server: Diseño/Arqui
- feat/mineria + inventario (1 semana)
  - picar los minerales
  - inventario (10 slots de stacks de 50)
  - conseguir la reconpensa de minar
- feat/granja (3 semanas)
  - logica animales (caminan)

#### lo que es para la proxima
- Camara: coali con la pared del mapa
- feat/cultivos -> implica tiempo
- feat/caverna
- feat/mapa
- QT COMPLETO: HANDSHAKE INCLUIDO
- Concurrencia Completa
- la idea era que tuvieramos regalos y enemigos




