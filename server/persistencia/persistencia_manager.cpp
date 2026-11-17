#include "persistencia_manager.h"

#include "persistencia/partida_toml_writer.h"

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <system_error>

#include <toml++/toml.hpp>

PersistenciaManager::PersistenciaManager(std::filesystem::path directorio):
    directorio(std::move(directorio)), archivoIndice(this->directorio / "partidas.toml") {
    cargarIndice();
}

void PersistenciaManager::cargarIndice() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!std::filesystem::exists(archivoIndice)) return;

    const toml::table root = toml::parse_file(archivoIndice.string());
    const auto* array = root.get_as<toml::array>("partidas");
    if (!array) return;

    for (const auto& node: *array) {
        const auto* table = node.as_table();
        if (!table) continue;

        const auto id = table->get_as<std::string>("id");
        const auto nombre = table->get_as<std::string>("nombre_granja");
        if (!id || !nombre) continue;

        PartidaIndexDTO partida;
        partida.id = id->get();
        partida.nombreGranja = nombre->get();
        if (const auto* usuarios = table->get_as<toml::array>("usuarios")) {
            for (const auto& usuario: *usuarios) {
                if (const auto valor = usuario.value<std::string>()) {
                    partida.usuarios.push_back(*valor);
                }
            }
        }
        partidas[partida.id] = std::move(partida);
    }
}

void PersistenciaManager::guardarIndice() const {
    std::filesystem::create_directories(directorio);

    toml::table root;
    toml::array lista;
    for (const auto& [id, partida]: partidas) {
        toml::table entrada;
        entrada.insert("id", id);
        entrada.insert("nombre_granja", partida.nombreGranja);
        toml::array usuarios;
        for (const auto& usuario: partida.usuarios) usuarios.push_back(usuario);
        entrada.insert("usuarios", std::move(usuarios));
        lista.push_back(std::move(entrada));
    }
    root.insert("partidas", std::move(lista));

    const auto temporal = archivoIndice.string() + ".tmp";
    {
        std::ofstream salida(temporal, std::ios::trunc);
        if (!salida) throw std::runtime_error("No se pudo crear el indice de partidas");
        salida << root;
        salida.flush();
        if (!salida) throw std::runtime_error("No se pudo escribir el indice de partidas");
    }

    std::error_code error;
    std::filesystem::rename(temporal, archivoIndice, error);
    if (error) {
        // En Windows rename no reemplaza un destino existente.
        std::filesystem::remove(archivoIndice, error);
        error.clear();
        std::filesystem::rename(temporal, archivoIndice, error);
    }
    if (error) {
        std::filesystem::remove(temporal);
        throw std::runtime_error("No se pudo actualizar el indice de partidas: " + error.message());
    }
}

void PersistenciaManager::registrarPartida(const std::string& id,
                                            const std::string& nombreGranja,
                                            const std::string& usuario) {
    std::lock_guard<std::mutex> lock(mtx);
    auto& partida = partidas[id];
    partida.id = id;
    partida.nombreGranja = nombreGranja;
    if (std::find(partida.usuarios.begin(), partida.usuarios.end(), usuario) == partida.usuarios.end()) {
        partida.usuarios.push_back(usuario);
    }
    guardarIndice();
}

void PersistenciaManager::registrarUsuario(const std::string& id,
                                            const std::string& usuario) {
    std::lock_guard<std::mutex> lock(mtx);
    const auto it = partidas.find(id);
    if (it == partidas.end()) return;
    auto& usuarios = it->second.usuarios;
    if (std::find(usuarios.begin(), usuarios.end(), usuario) == usuarios.end()) {
        usuarios.push_back(usuario);
        guardarIndice();
    }
}

void PersistenciaManager::persistirPartida(const PersistenciaDTO& partida) {
    if (partida.id.empty() || partida.id.find_first_of("/\\") != std::string::npos ||
        partida.id == "." || partida.id == "..") {
        throw std::invalid_argument("El ID de partida no es valido para usarlo como nombre de archivo");
    }

    PartidaTomlWriter writer;
    writer.write(partida, directorio / (partida.id + ".toml"));

    std::lock_guard<std::mutex> lock(mtx);
    auto& entrada = partidas[partida.id];
    entrada.id = partida.id;
    entrada.nombreGranja = partida.nombreGranja;
    entrada.usuarios = partida.usuarios;
    guardarIndice();
}

std::vector<PartidaIndexDTO> PersistenciaManager::obtenerPartidas() const {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<PartidaIndexDTO> resultado;
    resultado.reserve(partidas.size());
    for (const auto& [id, partida]: partidas) resultado.push_back(partida);
    return resultado;
}
