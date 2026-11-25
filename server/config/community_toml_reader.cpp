#include "community_toml_reader.h"

#include "server_constants.h"

#include <limits>

#include "commerce/shop_products.h"
#include "toml_helpers.h"

CommunityConfig CommunityTomlReader::read(const toml::table& root) const {
    CommunityConfig config;
    if (!root.contains(server_constants::COMMUNITY_CENTER_2)) return config;
    const auto& section = toml_helpers::tableAt(root.get(server_constants::COMMUNITY_CENTER_2), server_constants::COMMUNITY_CENTER_2);
    const auto number = [&](const char* name, uint32_t current, uint32_t maximum) {
        if (!section.contains(name)) return current;
        return static_cast<uint32_t>(toml_helpers::integer(section, name, maximum,
            std::string(server_constants::COMMUNITY_CENTER_3) + name));
    };
    config.moneyRequired = number(server_constants::MONEY_REQUIRED, config.moneyRequired, std::numeric_limits<uint32_t>::max());
    config.metalGoldReward = number(server_constants::METAL_GOLD_REWARD, config.metalGoldReward, std::numeric_limits<uint32_t>::max());
    config.seedReward = static_cast<uint16_t>(number(server_constants::SEED_REWARD, config.seedReward, std::numeric_limits<uint16_t>::max()));
    if (const auto* node = section.get(server_constants::REQUIREMENTS)) {
        const auto& requirements = toml_helpers::tableAt(node, server_constants::COMMUNITY_CENTER_REQUIREMENTS);
        for (const auto& [name, value] : requirements) {
            const auto item = shop_products::item(std::string(name.str()));
            if (!item || !config.requirements.contains(*item)) {
                toml_helpers::invalid(server_constants::COMMUNITY_CENTER_REQUIREMENTS, server_constants::UNKNOWN_PRODUCT);
            }
            config.requirements.at(*item) = static_cast<uint32_t>(toml_helpers::integer(
                requirements, name.str(), std::numeric_limits<uint32_t>::max(),
                server_constants::COMMUNITY_CENTER_REQUIREMENTS_2 + std::string(name.str())));
        }
    }
    config.validate();
    return config;
}
