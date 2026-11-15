#include <exception>
#include <iostream>
#include <string>
#include <stdexcept>

#include "config/game_toml_reader.h"
#include "game/game_manager.h"
#include "network/acceptor.h"
#include "socket.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0]
                  << " <puerto> [--world archivo.toml]\n";
        return 1;
    }

    try {
        std::string worldPath;

        for (int i = 2; i < argc; ++i) {
            const std::string argument = argv[i];

            if (argument == "--world") {
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
        }

        GameManager manager(
            world.config.dungeon,
            world.config.player,
            world.farm,
            world.config.town
        );

        Socket listener(argv[1]);
        Acceptor acceptor(listener, manager);
        acceptor.start();

        std::cout << "Servidor escuchando en "
                  << argv[1] << '\n';

        std::string line;
        while (std::getline(std::cin, line) && line != "q") {
        }

        acceptor.stop();
        acceptor.join();
        manager.stopAll();

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error del servidor: "
                  << e.what() << "\n";
        return 1;
    }
}
