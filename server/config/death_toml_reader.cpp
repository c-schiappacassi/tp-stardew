#include "config/death_toml_reader.h"

#include <limits>

#include "config/toml_helpers.h"

DeathConfig DeathTomlReader::read(const toml::table& root) const {
    using toml_helpers::integer;
    using toml_helpers::tableAt;

    const auto& death = tableAt(root.get("death"), "death");

    DeathConfig config;
    config.goldDrop = static_cast<uint32_t>(integer(
        death, "gold_drop", std::numeric_limits<uint32_t>::max(),
        "death.gold_drop"));

    return config;
}
