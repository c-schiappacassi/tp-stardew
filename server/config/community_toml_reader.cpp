#include "community_toml_reader.h"

#include <limits>

#include "commerce/shop_products.h"
#include "toml_helpers.h"

CommunityConfig CommunityTomlReader::read(const toml::table& root) const {
    CommunityConfig config;
    if (!root.contains("community_center")) return config;
    const auto& section = toml_helpers::tableAt(root.get("community_center"), "community_center");
    const auto number = [&](const char* name, uint32_t current, uint32_t maximum) {
        if (!section.contains(name)) return current;
        return static_cast<uint32_t>(toml_helpers::integer(section, name, maximum,
            std::string("community_center.") + name));
    };
    config.moneyRequired = number("money_required", config.moneyRequired, std::numeric_limits<uint32_t>::max());
    config.metalGoldReward = number("metal_gold_reward", config.metalGoldReward, std::numeric_limits<uint32_t>::max());
    config.seedReward = static_cast<uint16_t>(number("seed_reward", config.seedReward, std::numeric_limits<uint16_t>::max()));
    if (const auto* node = section.get("requirements")) {
        const auto& requirements = toml_helpers::tableAt(node, "community_center.requirements");
        for (const auto& [name, value] : requirements) {
            const auto item = shop_products::item(std::string(name.str()));
            if (!item || !config.requirements.contains(*item)) {
                toml_helpers::invalid("community_center.requirements", "unknown product");
            }
            config.requirements.at(*item) = static_cast<uint32_t>(toml_helpers::integer(
                requirements, name.str(), std::numeric_limits<uint32_t>::max(),
                "community_center.requirements." + std::string(name.str())));
        }
    }
    config.validate();
    return config;
}
