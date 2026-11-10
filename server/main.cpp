#include <exception>
#include <iostream>
#include <optional>
#include <string>
#include <stdexcept>

// include prueba
#include  "config/game_toml_reader.h"

#include "farm/loading/farm_toml_reader.h"
#include "game/game_manager.h"
#include "network/acceptor.h"
#include "socket.h"


/*
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <puerto> [--farm archivo.toml]\n"; //por ahroa asi para probar farm
        return 1;
    }
    try {
        std::string farmPath;
        for (int i = 2; i < argc; ++i) {
            const std::string argument = argv[i];
            if (argument == "--farm") {
                if (!farmPath.empty() || i + 1 == argc || std::string(argv[i + 1]).starts_with("--")) {
                    throw std::invalid_argument("--farm requiere un unico archivo TOML");
                }
                farmPath = argv[++i];
            } else {
                throw std::invalid_argument("Argumento desconocido: " + argument);
            }
        }
        std::optional<FarmLayout> farm;
        if (!farmPath.empty()) farm = FarmTomlReader{}.read(farmPath);
        GameManager manager({}, {}, farm);

        Socket listener(argv[1]);
        Acceptor acceptor(listener, manager);
        acceptor.start();
        std::cout << "Servidor escuchando en " << argv[1] << '\n';

        std::string line;
        while (std::getline(std::cin, line) && line != "q") {
        }

        acceptor.stop();
        acceptor.join();
        manager.stopAll();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error del servidor: " << e.what() << "\n";
        return 1;
    }
}
*/


#include <exception>
#include <iostream>
#include <optional>
#include <string>
#include <stdexcept>

#include "config/game_toml_reader.h"
#include "farm/loading/farm_toml_reader.h"
#include "game/game_manager.h"
#include "network/acceptor.h"
#include "socket.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0]
                  << " <puerto> [--farm archivo.toml]"
                  << " [--world archivo.toml]\n";
        return 1;
    }

    try {
        std::string farmPath;
        std::string worldPath;

        for (int i = 2; i < argc; ++i) {
            const std::string argument = argv[i];

            if (argument == "--farm") {
                if (!farmPath.empty() || i + 1 == argc ||
                    std::string(argv[i + 1]).starts_with("--")) {
                    throw std::invalid_argument(
                        "--farm requiere un unico archivo TOML");
                }
                farmPath = argv[++i];

            } else if (argument == "--world") {
                if (!worldPath.empty() || i + 1 == argc ||
                    std::string(argv[i + 1]).starts_with("--")) {
                    throw std::invalid_argument(
                        "--world requiere un unico archivo TOML");
                }
                worldPath = argv[++i];

            } else {
                throw std::invalid_argument(
                    "Argumento desconocido: " + argument);
            }
        }

        WorldData world;

        if (!worldPath.empty()) {
            world = GameTomlReader{}.read(worldPath);

            std::cout << "Configuracion general cargada.\n";
            std::cout << "Oro perdido al morir: "
                      << world.config.death.goldDrop << '\n';
            std::cout << "Productos de Robin: "
                      << world.config.robin.products.size() << '\n';
        }

        if (!farmPath.empty()) {
            world.farm = FarmTomlReader{}.read(farmPath, world.config.farm);
        }

        GameManager manager(
            world.config.dungeon,
            world.config.player,
            world.farm
        );

        Socket listener(argv[1]);
        Acceptor acceptor(listener, manager);
        acceptor.start();

        std::cout << "Servidor escuchando en " << argv[1] << '\n';

        std::string line;
        while (std::getline(std::cin, line) && line != "q") {
        }

        acceptor.stop();
        acceptor.join();
        manager.stopAll();

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error del servidor: " << e.what() << "\n";
        return 1;
    }
}
