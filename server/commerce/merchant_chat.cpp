#include "merchant_chat.h"

#include <algorithm>
#include <limits>
#include <set>
#include <stdexcept>

#include "farm/decorations/decoration.h"
#include "player/player.h"
#include "shop_products.h"
#include "world/world_manager.h"

namespace {
std::optional<uint32_t> priceOf(const std::vector<ShopItemPrice>& prices, ItemId item) {
    for (const auto& entry : prices) {
        if (entry.item == item) return entry.price;
    }
    return std::nullopt;
}

std::optional<ItemId> cropItem(CropType type) {
    switch (type) {
        case CropType::Strawberry: return ItemId::Strawberry;
        case CropType::Scallion: return ItemId::Scallion;
        case CropType::Potato: return ItemId::Potato;
        case CropType::Pumpkin: return ItemId::Pumpkin;
    }
    return std::nullopt;
}

std::string success(const TradeCommand& command, uint32_t price) {
    return std::string(command.action == TradeAction::Buy ? "Compraste " : "Vendiste ") +
        std::to_string(command.quantity) + " " + command.product + " por " +
        std::to_string(price) + " de oro.";
}
}

MerchantChat::MerchantChat(WorldManager& world, const MarketConfig& config) : world(world), config(config) {
    std::set<ItemId> registered;
    std::size_t rotating = 0;
    for (const auto& product : config.robin.products) {
        if (!registered.insert(product.item).second ||
            (product.item != ItemId::Chest && !Decoration::isDecoration(product.item)) ||
            product.rotates != (product.item != ItemId::Chest)) {
            throw std::invalid_argument("Catalogo de Robin invalido");
        }
        if (product.rotates) ++rotating;
    }
    if (config.robin.dailyRotationCount > rotating) {
        throw std::invalid_argument("Rotacion de Robin mayor al catalogo");
    }
    rotateCatalog();
}

void MerchantChat::rotateCatalog() {
    robinCatalog.clear();
    std::vector<RobinProductConfig> rotating;
    for (const auto& product : config.robin.products) {
        if (product.rotates) rotating.push_back(product);
        else robinCatalog.push_back(product);
    }
    for (std::size_t i = 0; i < config.robin.dailyRotationCount; ++i) {
        robinCatalog.push_back(rotating[(rotation + i) % rotating.size()]);
    }
    if (!rotating.empty()) rotation = (rotation + config.robin.dailyRotationCount) % rotating.size();
}

void MerchantChat::finishDay() {
    conversations.clear();
    rotateCatalog();
}

void MerchantChat::onDisconnect(uint16_t playerId) noexcept { conversations.erase(playerId); }

std::optional<std::vector<ChatMessageData>> MerchantChat::interact(
    const ChatCommandContext& context, Direction direction) {
    if (context.dayEnded || !context.player.canAct()) return std::nullopt;
    const auto npc = world.nearbyMerchant(context.player, direction);
    if (!npc) return std::nullopt;
    conversations.insert_or_assign(context.playerId, Conversation{*context.player.getInterior(), *npc});
    const std::string name{shop_products::merchantName(npc->id)};
    context.player.faceTarget(npc->pos);
    return std::vector<ChatMessageData>{
        {name, "Hola, " + context.playerName + ". Usa /consultar_catalogo para ver mis productos y precios."},
        {name, "Comandos: /consultar_catalogo | /comprar producto cantidad | /vender producto cantidad"}
    };
}

std::vector<ChatMessageData> MerchantChat::catalog(NpcId merchant) const {
    const std::string name{shop_products::merchantName(merchant)};
    std::vector<ChatMessageData> messages{{name, "Catalogo (precios por unidad en oro):"}};
    const auto add = [&](std::string_view product, uint32_t price, bool buys) {
        messages.push_back({name, std::string(buys ? "Compro " : "Vendo ") + std::string(product) +
                                  ": " + std::to_string(price)});
    };
    const auto& farmConfig = world.getFarm().getConfig();
    if (merchant == NpcId::Robin) {
        for (const auto& entry : robinCatalog) add(shop_products::name(entry.item), entry.price, false);
    } else if (merchant == NpcId::Clint) {
        for (const auto& entry : config.minerals) add(shop_products::name(entry.item), entry.price, true);
    } else if (merchant == NpcId::Pierre) {
        for (const auto& entry : config.seeds) add(shop_products::name(entry.item), entry.price, false);
        for (const auto& crop : farmConfig.crops) {
            const auto item = cropItem(crop.type);
            if (item) add(shop_products::name(*item), crop.basePrice, true);
        }
        for (const auto& animal : farmConfig.animals) {
            add(animal.species == AnimalSpecies::Cow ? "leche" : "huevo", animal.productPrice, true);
        }
        messages.push_back({name, "Cultivos: calidad base x1, buena x1.25, excelente x2, increible x4, perfecta x8."});
    } else if (merchant == NpcId::Marnie) {
        for (const auto& animal : farmConfig.animals) {
            const auto product = animal.species == AnimalSpecies::Cow ? "vacas" : "gallinas";
            add(product, animal.basePrice, false);
            add(product, animal.basePrice, true);
        }
    }
    return messages;
}

std::string MerchantChat::buy(Player& player, NpcId merchant, const TradeCommand& command) {
    if (merchant == NpcId::Marnie) {
        const auto species = shop_products::animal(command.product);
        for (const auto& definition : world.getFarm().getConfig().animals) {
            if (!species || definition.species != *species) continue;
            const uint64_t total = static_cast<uint64_t>(definition.basePrice) * command.quantity;
            if (total > player.getLocalData().gold) return "No tenes suficiente oro.";
            if (!world.buyAnimals(*species, command.quantity)) {
                return "No hay espacio en la granja o se supera el limite de animales.";
            }
            player.spendGold(static_cast<uint32_t>(total));
            return success(command, static_cast<uint32_t>(total));
        }
        return "No vendo ese animal.";
    }
    const auto item = shop_products::item(command.product);
    std::optional<uint32_t> unitPrice;
    if (item && merchant == NpcId::Pierre) unitPrice = priceOf(config.seeds, *item);
    if (item && merchant == NpcId::Robin) {
        for (const auto& product : robinCatalog) {
            if (product.item == *item) unitPrice = product.price;
        }
    }
    if (!unitPrice) return "No vendo ese producto. Consulta mi catalogo.";
    const uint64_t total = static_cast<uint64_t>(*unitPrice) * command.quantity;
    if (total > player.getLocalData().gold) return "No tenes suficiente oro.";
    if (!player.buyItem({*item, command.quantity, Quality::None}, static_cast<uint32_t>(total))) {
        return "No hay espacio suficiente en el inventario.";
    }
    return success(command, static_cast<uint32_t>(total));
}

std::string MerchantChat::sell(Player& player, NpcId merchant, const TradeCommand& command) {
    const auto data = player.getLocalData();
    const uint64_t goldSpace = std::numeric_limits<uint32_t>::max() - data.gold;
    if (merchant == NpcId::Marnie) {
        const auto species = shop_products::animal(command.product);
        for (const auto& definition : world.getFarm().getConfig().animals) {
            if (!species || definition.species != *species) continue;
            const uint64_t total = static_cast<uint64_t>(definition.basePrice) * command.quantity;
            if (total > goldSpace) return "La venta superaria el limite de oro.";
            if (!world.sellAnimals(*species, command.quantity)) return "No hay suficientes animales en la granja.";
            player.addGold(static_cast<uint32_t>(total));
            return success(command, static_cast<uint32_t>(total));
        }
        return "No compro ese animal.";
    }
    const auto item = shop_products::item(command.product);
    std::optional<uint32_t> basePrice;
    bool crop = false;
    if (item && merchant == NpcId::Clint) basePrice = priceOf(config.minerals, *item);
    if (item && merchant == NpcId::Pierre) {
        for (const auto& definition : world.getFarm().getConfig().crops) {
            if (cropItem(definition.type) == item) {
                basePrice = definition.basePrice;
                crop = true;
            }
        }
        for (const auto& definition : world.getFarm().getConfig().animals) {
            const auto product = definition.species == AnimalSpecies::Cow ? ItemId::Milk : ItemId::Egg;
            if (*item == product) basePrice = definition.productPrice;
        }
    }
    if (!basePrice) return "No compro ese producto.";
    std::vector<std::pair<uint8_t, uint16_t>> quantities;
    uint16_t remaining = command.quantity;
    uint64_t total = 0;
    for (const auto& [slot, stack] : data.items) {
        if (stack.item_id != *item || remaining == 0) continue;
        const auto unitPrice = shop_products::salePrice(*basePrice, crop ? stack.qual : Quality::None);
        if (!unitPrice) return "El producto tiene una calidad desconocida.";
        const auto quantity = std::min(remaining, stack.quantity);
        total += *unitPrice * quantity;
        quantities.emplace_back(slot, quantity);
        remaining -= quantity;
    }
    if (remaining != 0) return "No tenes suficiente cantidad de ese producto.";
    if (total > goldSpace) return "La venta superaria el limite de oro.";
    if (!player.sellItems(quantities, static_cast<uint32_t>(total))) return "No se pudo completar la venta.";
    return success(command, static_cast<uint32_t>(total));
}

std::vector<ChatMessageData> MerchantChat::execute(
    const ChatCommandContext& context, const TradeCommand& command) {
    if (context.dayEnded || !context.player.canAct()) {
        conversations.erase(context.playerId);
        return {{"Servidor", "No podes comerciar en este momento."}};
    }
    const auto conversation = conversations.find(context.playerId);
    if (conversation == conversations.end()) {
        return {{"Servidor", "Acercate a un comerciante e interactua antes de comerciar."}};
    }
    const auto npc = world.nearbyMerchant(context.player);
    const auto& active = conversation->second;
    if (context.player.getInterior() != active.location || !npc || npc->id != active.npc.id ||
        npc->pos.getX() != active.npc.pos.getX() || npc->pos.getY() != active.npc.pos.getY()) {
        conversations.erase(conversation);
        return {{"Servidor", "Te alejaste del comerciante. Acercate e interactua nuevamente."}};
    }
    if (command.action == TradeAction::Catalog) return catalog(npc->id);
    if (command.quantity == 0) return {{"Servidor", "La cantidad debe ser mayor a cero."}};
    const auto response = command.action == TradeAction::Buy
        ? buy(context.player, npc->id, command) : sell(context.player, npc->id, command);
    return {{std::string{shop_products::merchantName(npc->id)}, response}};
}
