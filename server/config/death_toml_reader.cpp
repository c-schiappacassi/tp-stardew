#include "config/death_toml_reader.h"

#include "server_constants.h"

#include <limits>

#include "config/toml_helpers.h"

DeathConfig DeathTomlReader::read(const toml::table& root) const {
    using toml_helpers::integer;
    using toml_helpers::tableAt;

    const auto& death = tableAt(root.get(server_constants::DEATH), server_constants::DEATH);

    DeathConfig config;
    config.goldDrop = static_cast<uint32_t>(integer(
        death, server_constants::GOLD_DROP, std::numeric_limits<uint32_t>::max(),
        server_constants::DEATH_GOLD_DROP));

    return config;
}
