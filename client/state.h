#pragma once

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "dtos.h"
#include "game_time.h"

// TODO:
class State {
private:
    std::map<std::string, PlayerData> players;
    std::vector<ChatMessageData> chat_session_history;
    PlayerDataLocal local;
    std::set<ItemId> ready_harvest;
    uint16_t local_time = game_time::START_MINUTES;
    std::unordered_map<CropType, ItemId> crop_to_item;
    bool day_confirmation = false;

    void refresh_alerts() {
        ready_harvest.clear();
        const auto* farm = std::get_if<FarmData>(&local.zoneData);
        if (!farm) {
            return;
        }
        for (const CropData& crop : farm->crops) {
            if (crop.state != CropState::Ready) {
                continue;
            }
            if (const auto it = crop_to_item.find(crop.type); it != crop_to_item.end()) {
                ready_harvest.insert(it->second);
            }
        }
    }

public:
    State() {
        // todo esto es para los alerts, abril dijo que en realidad se hace con el concepto de "Subscriber", pero que acá es overkill
        crop_to_item.emplace(CropType::Potato, ItemId::Potato);
        crop_to_item.emplace(CropType::Pumpkin, ItemId::Pumpkin);
        crop_to_item.emplace(CropType::Scallion, ItemId::Scallion);
        crop_to_item.emplace(CropType::Strawberry, ItemId::Strawberry);
    }

    void update(const ServerStatus& s) {
        players = s.players;
        local = s.localPlayer;
        day_confirmation = s.dayConfirmationRequired;
        local_time = s.timeOfDayMinutes;
        chat_session_history.insert(
            chat_session_history.end(),
            s.localPlayer.newMessages.begin(),
            s.localPlayer.newMessages.end()
        );
        refresh_alerts();
    }

    const std::map<std::string, PlayerData>& get_players() const { return players; }
    const PlayerDataLocal& get_local() const { return local; }
    const ZoneData& get_local_zone() const { return local.zoneData; }
    const uint16_t& get_local_time() const { return local_time; }
    const PlayerData* find_player(const std::string& name) const {
        auto it = players.find(name);
        return it == players.end() ? nullptr : &it->second;
    }
    bool day_confirmation_required() const { return day_confirmation; }
    const std::vector<ChatMessageData>& get_chat_history() const { return chat_session_history; }
    const std::set<ItemId>& get_alerts() const { return ready_harvest; }
};