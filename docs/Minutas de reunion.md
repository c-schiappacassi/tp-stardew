# 1er Reunion 13-10-2026
### Pre-reunion

| Rol   | Alumno    | Función    |
| :---  | :---      | :---      |
|A      |           |implementa la parte de SDL del cliente|
|B      |           |implementa la parte de QT del cliente (con editor)|
|C      |           |implementa la parte de comunicación entre cliente y servidor, threads, protocolo, testing y tareas extras (persistencia, configuración, etc)|
|D      |           |implementa la parte de lógica del servidor|

### Preguntas
- "Para jugar con amigos, cada partida contará con un id único de invitación, que se le enviará a los demás jugadores que desean participar"
    
    --> ese id es el id de partida? es un token? si es token se regenera en el momento o siempre es el mismo?

    abril dice:
    "criterio propio, no se la compliquen, que cumpla el enunciado y listo. token es mucho quilombo"

- La partida multiplayer no tiene limite de jugadores entonces?
    
    --> cuatro como en ele juego real?

    abril dice:
    "si, como la del juego real" 

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
    - cheats (soporte de protocolo)
- diseño de implementación  (reunion mínimo C y D)
    - cant hilos, clases, modelado de datos, seguir KISS, composición por sobre la herencia, etc...
- conexión cliente-servidor simple funcionando para tener de base. 
- SDL y QT: investiguen en la semana (QT juga con inputs) (SDL pp haga algo y se pueda interactuar)
