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

//Queda pendiente por charlar: MoveDTO, InteractDTO, DonateDTO, ChatDTO, UseItemCommand, RequestMultiplayerCode
//using Command = std::variant<MoveDTO, InteractDTO, DonateDTO, ChatDTO,UseItemCommand, RequestMultiplayerCode,DisconnectCommand>;
using Command = std::variant<DisconnectCommand>;

struct PlayerCommand {
    uint16_t playerId;
    Command command;
};

// no se soi mantener el variant todavia pero es la solucion que encontre para no tener un struct con muchos campos y que esten todos vacios menos 1 (le voy a preguntar a abril el martes)

//queda pendiente por charlar: hatMessageDTO, MultiplayerCodeDTO
//using OutgoingMessage = std::variant<StatusDTO, ChatMessageDTO, MultiplayerCodeDTO>;
using OutgoingMessage = std::variant<StatusDTO>;

// lo de abajo es como establecer un "sinonimo" y uso eso en todo el codigo
using CommandQueue = Queue<PlayerCommand>;
using OutQueue = Queue<OutgoingMessage>;

struct GameSession {
    StatusDTO status;
    std::string gameId;
    uint16_t playerId=0;
    uint8_t worldId=0;
    std::shared_ptr<CommandQueue> input; // si status != OK --> nullptr
};

#endif
