#include "player_chat.h"

#include <algorithm>
#include <exception>
#include <utility>

PlayerChat::PlayerChat(ChatServices services): services(std::move(services)) {}

bool PlayerChat::interact(PlayerRecord& sender, Direction direction, bool dayEnded) const {
    if (!services.merchants || dayEnded || !sender.player.canAct() ||
        direction > Direction::Right) return false;
    if (direction == Direction::None) direction = sender.player.getPublicData().dir;
    const ChatCommandContext context{sender.id, sender.name, sender.player, dayEnded};
    try {
        const auto responses = services.merchants->interact(context, direction);
        if (!responses) return false;
        sender.chatMessages.insert(sender.chatMessages.end(), responses->begin(), responses->end());
    } catch (const std::exception&) {
        reply(sender, "No se pudo iniciar la conversacion. Intenta nuevamente.");
    }
    return true;
}

void PlayerChat::onDisconnect(uint16_t playerId) const noexcept {
    if (services.merchants) services.merchants->onDisconnect(playerId);
    if (services.cheats) services.cheats->onDisconnect(playerId);
}

void PlayerChat::reply(PlayerRecord& sender, const std::string& text) {
    sender.chatMessages.push_back({"Servidor", text});
}

void PlayerChat::listCheats(PlayerRecord& sender) const {
    reply(sender, "/cheat vida N | /cheat energia N (0 a 65535)");
    reply(sender, "/cheat agregar_oro N | /cheat quitar_oro N (1 a 4294967295)");
    reply(sender, "/cheat completar_bundle identificador | /cheat completar_bundles");
    reply(sender, "/cheat siguiente_nivel | /cheat cuidar_cultivos | /cheat cuidar_animales");
    if (!services.cheats) reply(sender, "Los cheats todavia no estan disponibles.");
}

void PlayerChat::command(std::map<uint16_t, PlayerRecord>& players, PlayerRecord& sender,
                         const std::string& text, bool dayEnded) const {
    const auto parsed = ChatCommandParser::parse(text);
    if (const auto* error = std::get_if<ChatCommandError>(&parsed)) {
        reply(sender, error->text);
        return;
    }
    if (std::holds_alternative<ChatCheats>(parsed)) {
        listCheats(sender);
        return;
    }
    if (std::holds_alternative<ChatPlayers>(parsed)) {
        reply(sender, "Jugadores conectados:");
        for (const auto& [id, record] : players) {
            if (record.connected && record.output) reply(sender, record.name);
        }
        return;
    }

    const ChatCommandContext context{sender.id, sender.name, sender.player, dayEnded};
    std::vector<ChatMessageData> responses;
    try {
        if (const auto* request = std::get_if<TradeCommand>(&parsed)) {
            if (!services.merchants) {
                reply(sender, "El comercio todavia no esta disponible.");
                return;
            }
            if (dayEnded || !sender.player.canAct()) {
                reply(sender, "No podes comerciar en este momento.");
                return;
            }
            responses = services.merchants->execute(context, *request);
        } else if (const auto* request = std::get_if<CheatCommand>(&parsed)) {
            if (!services.cheats) {
                reply(sender, "Los cheats todavia no estan disponibles.");
                return;
            }
            responses = services.cheats->execute(context, *request);
        }
    } catch (const std::exception&) {
        reply(sender, "No se pudo ejecutar el comando. Intenta nuevamente.");
        return;
    }
    sender.chatMessages.insert(sender.chatMessages.end(), responses.begin(), responses.end());
}

void PlayerChat::privateMessage(std::map<uint16_t, PlayerRecord>& players,PlayerRecord& sender, const std::string& text) {
    PlayerRecord* recipient = nullptr;
    std::size_t body = std::string::npos;
    const auto separator = text.find(' ');
    if (separator == std::string::npos || separator == 1) {
        reply(sender, "Uso: @usuario mensaje");
        return;
    }
    for (auto& [id, record] : players) {
        const auto end = 1 + record.name.size();
        if (text.compare(1, record.name.size(), record.name) == 0 &&
            (end == text.size() || (end < text.size() && text[end] == ' ')) &&
            (!recipient || record.name.size() > recipient->name.size())) {
            recipient = &record;
            body = text.find_first_not_of(' ', end);
        }
    }
    if (recipient && body == std::string::npos) {
        reply(sender, "Uso: @usuario mensaje");
        return;
    }
    if (!recipient || !recipient->connected || !recipient->output) {
        reply(sender, "El jugador no existe o no esta conectado");
        return;
    }
    recipient->chatMessages.push_back({sender.name, text});
    if (recipient->id != sender.id) sender.chatMessages.push_back({sender.name, text});
}

void PlayerChat::handle(std::map<uint16_t, PlayerRecord>& players, uint16_t senderId,
                        const std::string& text, bool dayEnded) const {
    const auto senderIt = players.find(senderId);
    if (senderIt == players.end() || !senderIt->second.connected || !senderIt->second.output) return;
    auto& sender = senderIt->second;
    if (text.size() > MAX_LENGTH) {
        reply(sender, "El mensaje no puede superar los 100 caracteres");
        return;
    }
    if (std::any_of(text.begin(), text.end(), [](unsigned char ch) {
            return ch < 32 || ch == 127;
        })) {
        reply(sender, "El mensaje contiene caracteres de control");
        return;
    }
    const auto first = text.find_first_not_of(' ');
    if (first == std::string::npos) return;
    const auto last = text.find_last_not_of(' ');
    const auto message = text.substr(first, last - first + 1);
    if (message.front() == '@') {
        privateMessage(players, sender, message);
        return;
    }
    if (message.front() == '/') {
        command(players, sender, message, dayEnded);
        return;
    }
    for (auto& [id, record] : players) {
        if (record.connected && record.output) record.chatMessages.push_back({sender.name, message});
    }
}
