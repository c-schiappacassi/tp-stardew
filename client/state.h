#pragma once

#include <vector>
#include "dtos.h"

// TODO
class State {
private:
    std::map<std::string, PlayerData> players;
    std::vector<ChatMessageData> chat_session_history;
    PlayerDataLocal local;
    bool day_confirmation = false; 
public:
    void update(const ServerStatus& s) { 
        players = s.players; 
        local = s.localPlayer; 
        day_confirmation = s.dayConfirmationRequired; 
        chat_session_history.insert(
            chat_session_history.end(),
            s.localPlayer.newMessages.begin(),
            s.localPlayer.newMessages.end()
        );
    }
    const std::map<std::string, PlayerData>& get_players() const { return players; }
    const PlayerDataLocal& get_local() const { return local; }
    const ZoneData& get_local_zone() const { return local.zoneData; }
    const PlayerData* find_player(const std::string& name) const {
        auto it = players.find(name);
        return it == players.end() ? nullptr : &it->second;
    }
    bool day_confirmation_required() const { return day_confirmation; }
    const std::vector<ChatMessageData>& get_chat_history() const { return chat_session_history; }
};