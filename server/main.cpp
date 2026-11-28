#include "server_constants.h"

#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <stdexcept>

#include "config/game_toml_reader.h"
#include "config/interior_toml_reader.h"
#include "game/game_manager.h"
#include "network/acceptor.h"
#include "socket.h"

static std::string interiorsTomlPath() {
    for (auto directory = std::filesystem::current_path();; directory = directory.parent_path()) {
        const auto candidate = directory / server_constants::INTERIORS_TOML_PATH;
        if (std::filesystem::is_regular_file(candidate))
            return candidate.string();
        if (directory == directory.root_path())
            break;
    }
    throw std::runtime_error(std::string(server_constants::INTERIORS_TOML_NOT_FOUND_PREFIX) +
                             server_constants::INTERIORS_TOML_PATH);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << server_constants::USAGE_PREFIX << argv[0] << server_constants::PORT_WORLD_FILE_TOML_SEPARATOR;
        return 1;
    }

    try {
        std::string worldPath;

        for (int i = 2; i < argc; ++i) {
            const std::string argument = argv[i];

            if (argument == server_constants::OPTION_WORLD) {
                if (!worldPath.empty() || i + 1 == argc ||
                    std::string(argv[i + 1]).starts_with(server_constants::OPTION_PREFIX)) {
                    throw std::invalid_argument(
                        argument + server_constants::REQUIRES_A_SINGLE_TOML_FILE_SUFFIX);
                    }

                worldPath = argv[++i];
            } else {
                throw std::invalid_argument(
                    server_constants::UNKNOWN_ARGUMENT_PREFIX + argument);
            }
        }

        WorldData world;

        if (!worldPath.empty()) {
            world = GameTomlReader{}.read(worldPath);
        }
        world.config.interiors = InteriorTomlReader{}.readFile(interiorsTomlPath());

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
