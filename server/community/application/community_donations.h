#pragma once

#include "community/domain/community_center.h"

#include "community_donation_result.h"

class PlayerManager;
class PlayerSnapshot;

class CommunityDonations {
    CommunityCenter& center;

    static CommunityDonationError validate(const PlayerSnapshot& player, uint32_t amount, bool actionsBlocked);

public:
    explicit CommunityDonations(CommunityCenter& center);
    static bool isInsideCenter(const PlayerSnapshot& player);

    CommunityDonationResult donateItem(PlayerManager& players, uint16_t playerId, ItemId item, uint32_t amount, bool actionsBlocked = false);
    CommunityDonationResult donateSlot(PlayerManager& players, uint16_t playerId, uint8_t slot, uint32_t amount, bool actionsBlocked = false);
    CommunityDonationResult donateCoins(PlayerManager& players, uint16_t playerId, uint32_t amount, bool actionsBlocked = false);
};
