#include "server_constants.h"

#include <exception>
#include <iostream>
#include <string>
#include <stdexcept>

#include "config/game_toml_reader.h"
#include "config/interior_toml_reader.h"
#include "game/game_manager.h"
#include "network/acceptor.h"
#include "socket.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << server_constants::USAGE_PREFIX << argv[0] << server_constants::PORT_WORLD_FILE_TOML_INTERIORS_FILE_TOML_SEPARATOR;
        return 1;
    }

    try {
        std::string worldPath;
        std::string interiorsPath;

        for (int i = 2; i < argc; ++i) {
            const std::string argument = argv[i];

            if (argument == server_constants::OPTION_WORLD || argument == server_constants::OPTION_INTERIORS) {
                auto& path = argument == server_constants::OPTION_WORLD ? worldPath : interiorsPath;
                if (!path.empty() || i + 1 == argc ||
                    std::string(argv[i + 1]).starts_with(server_constants::OPTION_PREFIX)) {
                    throw std::invalid_argument(
                        argument + server_constants::REQUIRES_A_SINGLE_TOML_FILE_SUFFIX);
                    }

                path = argv[++i];
            } else {
                throw std::invalid_argument(
                    server_constants::UNKNOWN_ARGUMENT_PREFIX + argument);
            }
        }

        WorldData world;

        if (!worldPath.empty()) {
            world = GameTomlReader{}.read(worldPath);
        }
        if (!interiorsPath.empty())
            world.config.interiors = InteriorTomlReader{}.readFile(interiorsPath);

        GameManager manager(world.config, world.farm);

        Socket listener(argv[1]);
        Acceptor acceptor(listener, manager);
        acceptor.start();

        std::cout << server_constants::SERVER_LISTENING_ON_PREFIX
                  << argv[1] << '\n';

        std::string line;
        while (std::getline(std::cin, line) && line != server_constants::Q) {
        }

        acceptor.stop();
        acceptor.join();
        manager.stopAll();

        return 0;

    } catch (const std::exception& e) {
        std::cerr << server_constants::SERVER_ERROR_PREFIX
                  << e.what() << server_constants::NEWLINE;
        return 1;
    }
}
