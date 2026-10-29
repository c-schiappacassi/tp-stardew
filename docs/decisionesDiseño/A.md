# SDL_HANDLER

### Posible rediseño de las clases más bajas

en principio todas las clases se auto dibujan, y solo las clases más bajas son las que hacen uso de SDL puro. Pero como no me gusto como quedo player_view.cpp estoy pensando en crear una clase o directamente una funcion aux que sea un dibujador generico "handler" o "artist". Que le lleguen los datos genericos, osea, ya le llega el punto, el ancho, el alto y la sprite y lo dibuja, no se si voy a poder hacer que sea tan generico como para que recorte la sprite tambien o si le mando el recorte.

Esto implica un rediseño de la forma de usar asset manager, asi que TBD