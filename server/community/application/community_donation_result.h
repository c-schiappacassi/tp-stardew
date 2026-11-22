#pragma once

#include <cstdint>
#include "protocol_types.h"

struct CommunityDonationResult {
    CommunityDonationError error = CommunityDonationError::None;
    uint32_t amount = 0;
};
