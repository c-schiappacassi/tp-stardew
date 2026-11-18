#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "dtos.h"

class Player;

struct ChatCommandContext {
    uint16_t playerId;
    const std::string& playerName;
    Player& player;
    bool dayEnded;
};

enum class TradeAction { Catalog, Buy, Sell };

struct TradeCommand {
    TradeAction action;
    std::string product;
    uint16_t quantity = 0;
};

enum class CheatAction {
    SetLife,
    SetEnergy,
    AddGold,
    RemoveGold,
    CompleteBundle,
    CompleteAllBundles,
    NextLevel,
    CareCrops,
    CareAnimals
};

struct CheatCommand {
    CheatAction action;
    uint32_t amount = 0;
    std::string bundle;
};

class MerchantChatService {
public:
    virtual ~MerchantChatService() = default;
    virtual std::optional<std::vector<ChatMessageData>> interact(const ChatCommandContext&,Direction) { return std::nullopt; }
    virtual std::vector<ChatMessageData> execute(const ChatCommandContext& context,const TradeCommand& command) = 0;
    virtual void onDisconnect(uint16_t) noexcept {}
};

class CheatChatService {
public:
    virtual ~CheatChatService() = default;
    virtual std::vector<ChatMessageData> execute(const ChatCommandContext& context,const CheatCommand& command) = 0;
    virtual void onDisconnect(uint16_t) noexcept {}
};

struct ChatServices {
    std::shared_ptr<MerchantChatService> merchants;
    std::shared_ptr<CheatChatService> cheats;
};
