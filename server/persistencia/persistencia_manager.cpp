#include "persistencia_manager.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <utility>

#include <toml++/toml.hpp>


PersistenciaManager::PersistenciaManager(
    const std::string& ruta)
    : rutaIndice(ruta) {
    cargarIndice();
}


void PersistenciaManager::cargarIndice() {
    partidas.clear();

    if (!std::filesystem::exists(rutaIndice)) {
        return;
    }

    toml::table archivo = toml::parse_file(rutaIndice);

    auto* lista = archivo["partidas"].as_array();

    if (lista == nullptr) {
        throw std::runtime_error(
            "El indice de partidas no es valido");
    }

    for (const auto& elemento : *lista) {
        const auto* tabla = elemento.as_table();

        if (tabla == nullptr) {
            throw std::runtime_error(
                "Partida invalida en el indice");
        }

        PartidaIndexDTO partida;

        auto id = (*tabla)["id"].value<std::string>();
        auto nombre = (*tabla)["nombreGranja"].value<std::string>();
        const auto* usuarios = (*tabla)["usuarios"].as_array();

        if (!id || !nombre || usuarios == nullptr) {
            throw std::runtime_error(
                "Faltan datos en una partida");
        }

        partida.id = *id;
        partida.nombreGranja = *nombre;

        for (const auto& usuario : *usuarios) {
            auto nombreUsuario = usuario.value<std::string>();

            if (!nombreUsuario) {
                throw std::runtime_error(
                    "Usuario invalido en el indice");
            }

            partida.usuarios.push_back(*nombreUsuario);
        }

        partidas.push_back(std::move(partida));
    }
}


void PersistenciaManager::guardarIndice() const {
    toml::table archivo;
    toml::array lista;

    for (const auto& partida : partidas) {
        toml::table tabla;

        tabla.insert("id", partida.id);
        tabla.insert("nombreGranja", partida.nombreGranja);

        toml::array usuarios;

        for (const auto& usuario : partida.usuarios) {
            usuarios.push_back(usuario);
        }

        tabla.insert("usuarios", std::move(usuarios));

        lista.push_back(std::move(tabla));
    }

    archivo.insert("partidas", std::move(lista));

    std::ofstream salida(rutaIndice, std::ios::trunc);

    if (!salida.is_open()) {
        throw std::runtime_error(
            "No se pudo abrir el indice para guardar");
    }

    salida << archivo;

    if (!salida) {
        throw std::runtime_error(
            "No se pudo escribir el indice de partidas");
    }
}


std::vector<PartidaIndexDTO>
PersistenciaManager::obtenerPartidas() const {
    return partidas;
}



void PersistenciaManager::registrarPartida(
    const std::string& id,
    const std::string& nombreGranja,
    const std::string& usuario) {

    for (const auto& partida : partidas) {
        if (partida.id == id) {
            throw std::runtime_error(
                "Ya existe una partida con ese ID");
        }
    }

    PartidaIndexDTO nuevaPartida;

    nuevaPartida.id = id;
    nuevaPartida.nombreGranja = nombreGranja;
    nuevaPartida.usuarios.push_back(usuario);

    partidas.push_back(std::move(nuevaPartida));

    guardarIndice();
}


void PersistenciaManager::registrarUsuario(
    const std::string& id,
    const std::string& usuario) {

    for (auto& partida : partidas) {
        if (partida.id == id) {

            auto it = std::find(
                partida.usuarios.begin(),
                partida.usuarios.end(),
                usuario);

            if (it != partida.usuarios.end()) {
                return;
            }

            partida.usuarios.push_back(usuario);

            guardarIndice();
            return;
        }
    }

    throw std::runtime_error(
        "No existe la partida indicada");
}
