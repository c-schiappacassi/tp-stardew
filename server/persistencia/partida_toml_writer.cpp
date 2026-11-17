#include "persistencia/partida_toml_writer.h"

#include <cstdint>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <system_error>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

#include <toml++/toml.hpp>

namespace {

template <typename Enum>
int64_t enumValue(Enum value) {
    return static_cast<int64_t>(static_cast<std::underlying_type_t<Enum>>(value));
}

int64_t integerValue(uint64_t value, const std::string& field) {
    if (value > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
        throw std::overflow_error("El valor de " + field + " no puede representarse en TOML");
    }
    return static_cast<int64_t>(value);
}

toml::table positionTable(const Position& position) {
    toml::table table;
    table.insert("x", position.getX());
    table.insert("y", position.getY());
    return table;
}

toml::table colorTable(const Color& color) {
    toml::table table;
    table.insert("r", static_cast<int64_t>(color.r));
    table.insert("g", static_cast<int64_t>(color.g));
    table.insert("b", static_cast<int64_t>(color.b));
    return table;
}

toml::table furnitureTable(const FurnitureData& furniture) {
    toml::table table;
    table.insert("id", integerValue(furniture.id, "mueble.id"));
    table.insert("posicion", positionTable(furniture.pos));
    table.insert("tipo", enumValue(furniture.type));
    table.insert("direccion", enumValue(furniture.dir));
    return table;
}

toml::table farmTable(const FarmData& farm) {
    toml::table table;

    toml::array crops;
    for (const auto& crop: farm.crops) {
        toml::table entry;
        entry.insert("id", integerValue(crop.id, "cultivo.id"));
        entry.insert("posicion", positionTable(crop.pos));
        entry.insert("tipo", enumValue(crop.type));
        entry.insert("estado", enumValue(crop.state));
        entry.insert("regado", crop.wattered);
        crops.push_back(std::move(entry));
    }
    table.insert("cultivos", std::move(crops));

    toml::array animals;
    for (const auto& animal: farm.animals) {
        toml::table entry;
        entry.insert("id", integerValue(animal.id, "animal.id"));
        entry.insert("posicion", positionTable(animal.pos));
        entry.insert("direccion", enumValue(animal.dir));
        entry.insert("tipo", enumValue(animal.type));
        entry.insert("estado", enumValue(animal.state));
        entry.insert("necesita_interaccion", animal.needs_interaction);
        animals.push_back(std::move(entry));
    }
    table.insert("animales", std::move(animals));

    toml::array soil;
    for (const auto& position: farm.tilledSoil) soil.push_back(positionTable(position));
    table.insert("tierra_labrada", std::move(soil));

    toml::array furniture;
    for (const auto& item: farm.furniture) furniture.push_back(furnitureTable(item));
    table.insert("muebles", std::move(furniture));
    return table;
}

toml::table combatTable(const DungeonCombatData& combat) {
    toml::table table;
    table.insert("visit_id", integerValue(combat.visitId, "combate.visit_id"));
    table.insert("habitacion", static_cast<int64_t>(combat.room));

    toml::array enemies;
    for (const auto& enemy: combat.enemies) {
        toml::table entry;
        entry.insert("id", integerValue(enemy.id, "enemigo.id"));
        entry.insert("posicion", positionTable(enemy.pos));
        entry.insert("direccion", enumValue(enemy.dir));
        entry.insert("tipo", enumValue(enemy.type));
        entry.insert("estado", enumValue(enemy.state));
        entry.insert("vida", static_cast<int64_t>(enemy.health));
        entry.insert("vida_maxima", static_cast<int64_t>(enemy.maxHealth));
        entry.insert("enfurecido", enemy.frenzied);
        entry.insert("accion", integerValue(enemy.action, "enemigo.accion"));
        enemies.push_back(std::move(entry));
    }
    table.insert("enemigos", std::move(enemies));

    toml::array projectiles;
    for (const auto& projectile: combat.projectiles) {
        toml::table entry;
        entry.insert("id", integerValue(projectile.id, "proyectil.id"));
        entry.insert("posicion", positionTable(projectile.pos));
        entry.insert("tipo", enumValue(projectile.type));
        projectiles.push_back(std::move(entry));
    }
    table.insert("proyectiles", std::move(projectiles));

    toml::array traps;
    for (const auto& trap: combat.traps) {
        toml::table entry;
        entry.insert("id", integerValue(trap.id, "trampa.id"));
        entry.insert("posicion", positionTable(trap.pos));
        entry.insert("direccion", enumValue(trap.dir));
        entry.insert("tipo", enumValue(trap.type));
        entry.insert("estado", enumValue(trap.state));
        entry.insert("rango", static_cast<int64_t>(trap.range));
        traps.push_back(std::move(entry));
    }
    table.insert("trampas", std::move(traps));

    toml::array loot;
    for (const auto& item: combat.loot) {
        toml::table entry;
        entry.insert("id", integerValue(item.id, "loot.id"));
        entry.insert("posicion", positionTable(item.pos));
        entry.insert("item", enumValue(item.item));
        entry.insert("cantidad", static_cast<int64_t>(item.quantity));
        loot.push_back(std::move(entry));
    }
    table.insert("botin", std::move(loot));
    return table;
}

toml::table interiorTable(const InteriorData& interior) {
    toml::table table;
    table.insert("interior_id", enumValue(interior.interior_id));
    table.insert("house_index", static_cast<int64_t>(interior.house_index));
    toml::array furniture;
    for (const auto& item: interior.furniture) furniture.push_back(furnitureTable(item));
    table.insert("muebles", std::move(furniture));
    return table;
}

toml::table playerTable(const PlayerDataLocal& player) {
    toml::table table;
    table.insert("nombre_personaje", player.characterName);
    table.insert("posicion", positionTable(player.pos));
    table.insert("estado", enumValue(player.state));
    table.insert("direccion", enumValue(player.dir));
    table.insert("modelo_pelo", static_cast<int64_t>(player.hairModel));
    table.insert("modelo_camisa", static_cast<int64_t>(player.shirtModel));
    table.insert("modelo_pantalon", static_cast<int64_t>(player.pantsModel));
    table.insert("color_pelo", colorTable(player.hairColor));
    table.insert("color_pantalon", colorTable(player.pantsColor));
    table.insert("zona", enumValue(player.currentZone));
    if (player.interior) {
        toml::table interior;
        interior.insert("id", enumValue(player.interior->interior_id));
        interior.insert("indice_casa", static_cast<int64_t>(player.interior->house_index));
        table.insert("interior", std::move(interior));
    }
    if (player.heldItem) table.insert("objeto_en_mano", enumValue(*player.heldItem));

    toml::array items;
    for (const auto& [slot, stack]: player.items) {
        toml::table entry;
        entry.insert("slot", static_cast<int64_t>(slot));
        entry.insert("item_id", enumValue(stack.item_id));
        entry.insert("cantidad", static_cast<int64_t>(stack.quantity));
        entry.insert("calidad", enumValue(stack.qual));
        items.push_back(std::move(entry));
    }
    table.insert("inventario", std::move(items));
    table.insert("slot_actual", static_cast<int64_t>(player.current_slot));
    table.insert("oro", integerValue(player.gold, "jugador.oro"));
    table.insert("vida", static_cast<int64_t>(player.health));
    table.insert("vida_maxima", static_cast<int64_t>(player.maxHealth));
    table.insert("energia", static_cast<int64_t>(player.energy));
    table.insert("energia_maxima", static_cast<int64_t>(player.maxEnergy));

    toml::table combat;
    combat.insert("skeletons", player.combatProgress.skeletons);
    combat.insert("vampires", player.combatProgress.vampires);
    combat.insert("orcs", player.combatProgress.orcs);
    combat.insert("skull", player.combatProgress.skull);
    table.insert("progreso_combate", std::move(combat));
    
    toml::table zoneData;
    std::visit([&zoneData](const auto& data) {
        using T = std::decay_t<decltype(data)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
            zoneData.insert("tipo", "ninguna");
        } else if constexpr (std::is_same_v<T, FarmData>) {
            zoneData.insert("tipo", "granja");
            zoneData.insert("datos", farmTable(data));
        } else if constexpr (std::is_same_v<T, InteriorData>) {
            zoneData.insert("tipo", "interior");
            zoneData.insert("datos", interiorTable(data));
        } else if constexpr (std::is_same_v<T, MineData>) {
            zoneData.insert("tipo", "mina");
            toml::table mine;
            mine.insert("nivel_actual", static_cast<int64_t>(data.current_level));
            toml::table level;
            level.insert("entrada", positionTable(data.current_level_data.entry));
            level.insert("salida", positionTable(data.current_level_data.exit));
            toml::array minerals;
            for (const auto& mineral: data.current_level_data.minerals) {
                toml::table entry;
                entry.insert("id", integerValue(mineral.id, "mineral.id"));
                entry.insert("posicion", positionTable(mineral.pos));
                entry.insert("tipo", enumValue(mineral.type));
                entry.insert("estado", enumValue(mineral.state));
                minerals.push_back(std::move(entry));
            }
            level.insert("minerales", std::move(minerals));
            level.insert("combate", combatTable(data.current_level_data.combat));
            mine.insert("nivel", std::move(level));
            zoneData.insert("datos", std::move(mine));
        }
    }, player.zoneData);
    table.insert("datos_zona", std::move(zoneData));
    return table;
}

toml::table nestedProgressTable(const PersistenciaDTO& partida) {
    toml::table root;
    for (const auto& [taskId, actions]: partida.progresoAccionesCentroComunitario) {
        toml::table task;
        for (const auto& [actionId, requirements]: actions) {
            toml::table action;
            for (const auto& [requirementId, amount]: requirements) {
                action.insert(requirementId, static_cast<int64_t>(amount));
            }
            task.insert(actionId, std::move(action));
        }
        root.insert(taskId, std::move(task));
    }
    return root;
}

toml::table idToUint64Table(const std::map<uint32_t, uint64_t>& values,
                            const std::string& field) {
    toml::table table;
    for (const auto& [id, value]: values) {
        table.insert(std::to_string(id), integerValue(value, field));
    }
    return table;
}

toml::table idToUint32Table(const std::map<uint32_t, uint32_t>& values) {
    toml::table table;
    for (const auto& [id, value]: values) {
        table.insert(std::to_string(id), static_cast<int64_t>(value));
    }
    return table;
}

toml::table idToBoolTable(const std::map<uint32_t, bool>& values) {
    toml::table table;
    for (const auto& [id, value]: values) table.insert(std::to_string(id), value);
    return table;
}

}  // namespace

void PartidaTomlWriter::write(const PersistenciaDTO& partida,
                              const std::filesystem::path& archivo) const {
    if (partida.id.empty()) throw std::invalid_argument("La partida no tiene ID");

    toml::table root;
    root.insert("id", partida.id);
    root.insert("nombre_granja", partida.nombreGranja);
    root.insert("mapa_id", static_cast<int64_t>(partida.mapaId));
    root.insert("dia_actual", static_cast<int64_t>(partida.diaActual));

    toml::array users;
    for (const auto& user: partida.usuarios) users.push_back(user);
    root.insert("usuarios", std::move(users));

    toml::table players;
    for (const auto& [user, player]: partida.jugadores) {
        players.insert(user, playerTable(player));
    }
    root.insert("jugadores", std::move(players));
    root.insert("granja", farmTable(partida.granja));

    toml::table cropsGrowth = idToUint64Table(partida.progresoCrecimientoCultivos,
                                              "progreso_crecimiento_cultivos");
    root.insert("progreso_crecimiento_cultivos", std::move(cropsGrowth));
    toml::table cropWater = idToUint64Table(partida.tiempoAguaRestanteCultivos,
                                            "tiempo_agua_restante_cultivos");
    root.insert("tiempo_agua_restante_cultivos", std::move(cropWater));
    root.insert("cultivos_cosechados", idToBoolTable(partida.cultivosCosechados));
    root.insert("progreso_produccion_animales",
                idToUint64Table(partida.progresoProduccionAnimales,
                                "progreso_produccion_animales"));
    root.insert("productos_disponibles_animales",
                idToUint32Table(partida.productosDisponiblesAnimales));
    root.insert("progreso_centro_comunitario", nestedProgressTable(partida));

    if (!archivo.parent_path().empty()) std::filesystem::create_directories(archivo.parent_path());
    const std::filesystem::path temporal = archivo.string() + ".tmp";
    {
        std::ofstream output(temporal, std::ios::trunc);
        if (!output) throw std::runtime_error("No se pudo crear el archivo de partida: " + temporal.string());
        output << root;
        output.flush();
        if (!output) {
            std::filesystem::remove(temporal);
            throw std::runtime_error("No se pudo escribir el archivo de partida: " + temporal.string());
        }
    }

    std::error_code error;
    std::filesystem::rename(temporal, archivo, error);
    if (error) {
        std::error_code removeError;
        std::filesystem::remove(archivo, removeError);
        error.clear();
        std::filesystem::rename(temporal, archivo, error);
    }
    if (error) {
        std::filesystem::remove(temporal);
        throw std::runtime_error("No se pudo reemplazar el archivo de partida: " + error.message());
    }
}
