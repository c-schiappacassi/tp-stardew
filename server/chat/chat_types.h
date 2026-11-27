#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "dtos.h"
#include "position.h"
#include "protocol_types.h"

class PlayerManager;
class MerchantChatService;
class CheatChatService;
class CommunityChatService;

// Commands
enum class CheatAction {
    SetLife,
    SetEnergy,
    AddGold,
    RemoveGold,
    CompleteBundle,
    CompleteAllBundles,
    NextLevel,
    CareCrops,
    CareAnimals,
    Heal,
    GiveItem,
    ClearSlot,
    SpawnAnimal,
    Warp,
    EndDay,
    SpawnEnemy,
    KillEnemy,
    KillEnemies,
    ListItems
};

struct CheatCommand {
    CheatAction action;
    uint32_t amount = 0;
    std::string target;
    Quality quality = Quality::None;
};

enum class TradeAction {
    Catalog,
    Buy,
    Sell
};

struct TradeCommand {
    TradeAction action;
    std::string product;
    uint16_t quantity = 0;
};

struct CommunityStatusCommand {
    std::string bundle;
};

struct CommunityRewardsCommand {};

struct DonateItemCommand {
    std::string product;
    uint32_t quantity;
};

struct DonateCoinsCommand {
    uint32_t quantity;
};

using CommunityCommand = std::variant<CommunityStatusCommand, CommunityRewardsCommand, DonateItemCommand, DonateCoinsCommand>;

enum class GiftAction { Offer, Accept, Reject, Cancel };

struct GiftCommand {
    GiftAction action;
    std::string player;
    uint16_t quantity = 1;
};

struct ChatCheats {};
struct ChatPlayers {};

struct ChatCommandError {
    std::string text;
};

using ParsedChatCommand = std::variant<
    ChatCheats,
    ChatPlayers,
    GiftCommand,
    TradeCommand,
    CommunityCommand,
    CheatCommand,
    ChatCommandError>;

// Context & Services
struct ChatCommandContext {
    uint16_t playerId;
    const std::string& playerName;
    PlayerManager& players;
    bool actionsBlocked;
    std::vector<Position> occupiedPositions{};
};

struct ChatServices {
    std::shared_ptr<MerchantChatService> merchants{};
    std::shared_ptr<CheatChatService> cheats{};
    std::shared_ptr<CommunityChatService> community{};
};

// Reply
struct CommunityDonationReply {
    CommunityDonationReceipt receipt;
    std::vector<ChatMessageData> messages;
};
