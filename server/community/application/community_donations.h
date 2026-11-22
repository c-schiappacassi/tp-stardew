#pragma once

#include "community/domain/community_center.h"
#include "community_donation_result.h"

class Player;

class CommunityDonations {
    CommunityCenter& center;

    static CommunityDonationError validate(const Player& player, uint32_t amount, bool actionsBlocked);

public:
    explicit CommunityDonations(CommunityCenter& center);
    static bool isInsideCenter(const Player& player);

    CommunityDonationResult donateItem(Player& player, ItemId item, uint32_t amount, bool actionsBlocked = false);
    CommunityDonationResult donateSlot(Player& player, uint8_t slot, uint32_t amount, bool actionsBlocked = false);
    CommunityDonationResult donateCoins(Player& player, uint32_t amount, bool actionsBlocked = false);
};
