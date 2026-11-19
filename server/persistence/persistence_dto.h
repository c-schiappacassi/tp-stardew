#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "dtos.h"

struct PersistenceDTO {
    std::string id;
    std::string farmName;
    uint8_t mapId = 0;
    uint32_t currentDay = 1;

    std::vector<std::string> users;
    std::map<std::string, PlayerDataLocal> players;

    FarmData farm;

    std::map<uint32_t, uint64_t> cropGrowthProgress;
    std::map<uint32_t, uint64_t> cropWaterRemainingTime;
    std::map<uint32_t, bool> harvestedCrops;

    std::map<uint32_t, uint64_t> animalProductionProgress;
    std::map<uint32_t, uint32_t> animalAvailableProducts;

    std::map<
        std::string,
        std::map<
            std::string,
            std::map<std::string, uint32_t>
        >
    > communityCenterActionProgress;

    // progresoCentroComunitario: id de tarea -> id de accion -> par (cantidad realizada, cantidad requerida)
    // como tenemos que guardar el progreso de las tareas hacemos que sea algo asi la estructura:
    // guardamos lo de enemys como enemys[tipo de enemy] -> {numero, numero} y esos numeros son {cantidad que mataron, cantidad necesaria}
    // despues igual decime que opinas de esto, no se si lo guardas de una forma distinta y mejor, tambien sirve que sea la misma estructura asi lo mantenemos asi

    // ejemplo de lo que pensaba por si no se entiende:
    // progresoAccionesCentroComunitario["matar_enemigos"]["vampiro"]["abril"] = 1;
};
