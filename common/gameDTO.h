#pragma once
#include <variant>

// MOCK para el proto

struct MoveDTO { int dx, dy; };
struct AttackDTO { int weapon_id; };
struct BuyDTO { int item_id; };
struct QuitDTO {};

using GameDTO = std::variant<MoveDTO, AttackDTO, BuyDTO, QuitDTO>;

struct serverStatus{};