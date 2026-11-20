#include "community_config.h"

#include <stdexcept>

#include "community_bundle.h"

void CommunityConfig::validate() const {
    if (requirements.size() != community_bundle::requiredItems.size() || moneyRequired == 0 || seedReward == 0) {
        throw std::invalid_argument("Invalid community center configuration");
    }
    for (const auto item : community_bundle::requiredItems) {
        const auto requirement = requirements.find(item);
        if (requirement == requirements.end() || requirement->second == 0) {
            throw std::invalid_argument("Invalid community center requirement");
        }
    }
}
