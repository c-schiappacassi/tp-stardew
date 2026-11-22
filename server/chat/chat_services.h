#pragma once

#include <memory>

class MerchantChatService;
class CheatChatService;
class CommunityChatService;

struct ChatServices {
    std::shared_ptr<MerchantChatService> merchants{};
    std::shared_ptr<CheatChatService> cheats{};
    std::shared_ptr<CommunityChatService> community{};
};
