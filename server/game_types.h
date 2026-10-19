#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include <cstdint>
#include <memory>
#include <variant>

#include "dtos.h"
#include "queue.h"


// o es mejor tener un dto?? (uso estos porque no tengo dtos para estas opciones)
struct UseItemCommand {};
struct RequestMultiplayerCode {};
struct DisconnectCommand {};

using Command = std::variant<MoveDTO, InteractDTO, DonateDTO, ChatDTO,
                             UseItemCommand, RequestMultiplayerCode,
                             DisconnectCommand>;

struct PlayerCommand {
    uint16_t playerId;
    Command command;
};

// no se soi mantener el variant todavia pero es la solucion que encontre para no tener un struct con muchos campos y que esten todos vacios menos 1 (le voy a preguntar a abril el martes)
using OutgoingMessage = std::variant<StatusDTO, ChatMessageDTO, MultiplayerCodeDTO>;

// lo de abajo es como establecer un "sinonimo" y uso eso en todo el codigo
using CommandQueue = Queue<PlayerCommand>;
using OutQueue = Queue<OutgoingMessage>;

struct GameSession {
    StatusDTO status;
    uint16_t playerId;
    uint8_t worldId;
    std::shared_ptr<CommandQueue> input; // si status != OK --> nullptr
};

#endif
