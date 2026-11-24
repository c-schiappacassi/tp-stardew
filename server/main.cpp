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
        std::cerr << "Usage: " << argv[0] << " <port> [--world file.toml]\n";
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
                        "--world requires a single TOML file");
                    }

                worldPath = argv[++i];
            } else {
                throw std::invalid_argument(
                    "Unknown argument: " + argument);
            }
        }

        WorldData world;

        if (!worldPath.empty()) {
            world = GameTomlReader{}.read(worldPath);
        }

        GameManager manager(world.config, world.farm);

        Socket listener(argv[1]);
        Acceptor acceptor(listener, manager);
        acceptor.start();

        std::cout << "Server listening on "
                  << argv[1] << '\n';

        std::string line;
        while (std::getline(std::cin, line) && line != "q") {
        }

        acceptor.stop();
        acceptor.join();
        manager.stopAll();

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Server error: "
                  << e.what() << "\n";
        return 1;
    }
}
