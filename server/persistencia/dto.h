#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "dtos.h"

// AGUS TE DEJO COMENTARIOS DE QUE ES CADA COSA!

struct PersistenciaDTO {
    // partida
    std::string id;
    std::string nombreGranja;
    uint8_t mapaId = 0;
    uint32_t diaActual = 1;

    // usuarios de la partida (solo los ids/nombres)
    std::vector<std::string> usuarios;

    // reutilizamos playerdata y ahi me mandas vestimenta, otro etc
    std::map<std::string, PlayerDataLocal> jugadores;

    // reutilizamos aca tmb
    FarmData granja;

    // estos te los explico tipo id -> conenido
    std::map<uint32_t, uint64_t> progresoCrecimientoCultivos;
    std::map<uint32_t, uint64_t> tiempoAguaRestanteCultivos;
    std::map<uint32_t, bool> cultivosCosechados;
    std::map<uint32_t, uint64_t> progresoProduccionAnimales;
    std::map<uint32_t, uint32_t> productosDisponiblesAnimales;

    // progresoCrecimientoCultivos: id de cultivo -> milisegundos acumulados
    // tiempoAguaRestanteCultivos: id de cultivo -> milisegundos restantes
    // cultivosCosechados: id de cultivo -> si ya fue cosechado
    // progresoProduccionAnimales: id de animal -> milisegundos acumulados
    // productosDisponiblesAnimales: id de animal -> productos acumulados

    std::map<
        std::string,
        std::map<
            std::string,
            std::map<std::string, uint32_t>
        >
    > progresoAccionesCentroComunitario;

    // progresoCentroComunitario: id de tarea -> id de accion -> par (cantidad realizada, cantidad requerida)
    // como tenemos que guardar el progreso de las tareas hacemos que sea algo asi la estructura:
    // guardamos lo de enemys como enemys[tipo de enemy] -> {numero, numero} y esos numeros son {cantidad que mataron, cantidad necesaria}
    // despues igual decime que opinas de esto, no se si lo guardas de una forma distinta y mejor, tambien sirve que sea la misma estructura asi lo mantenemos asi

    // te dejo un ejemplo de lo que pensaba por si no se entiende:
    // progresoAccionesCentroComunitario["matar_enemigos"]["vampiro"]["abril"] = 1;
};
