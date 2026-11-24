#pragma once
#include <cstdint>

const int SUCCESS = 0;
const int FAILURE = 1;

const unsigned int INITIAL_PLAYERS = 1;
const char SERVER_QUIT_CHAR = 'q';

const char* const CMD_LIST = "list";
const char* const CMD_CREATE = "create";
const char* const CMD_JOIN = "join";
const char* const CMD_END = "end";

const char* const CREATE_SUCCESS_MSG = "Game created successfully";
const char* const CREATE_FAILURE_MSG = "Game creation failed";
const char* const JOIN_SUCCESS_MSG = "Joined successfully";
const char* const JOIN_FAILURE_MSG = "Failed to join";

const char* const GAME_FULL_PREFIX = "Game full ";
const char* const GAME_FULL_SUFFIX = "...";

const char* const LIST_SEPARATOR = "/";
const char* const SPACE_SEPARATOR = " ";
const char* const NEXT_LINE = "\n";

const char* const UNEXPECTED_ERROR_MSG = "Unexpected error: ";
const char* const UNKNOWN_ERROR_MSG = "Unknown error";
const char* const SERVER_ERROR_MSG = "Server connection error: ";
const char* const SOCKET_ERROR_MSG = "Socket error: ";
const char* const UNKNOWN_OPERATOR_MSG = "Unknown operation";

const uint8_t PLAYER_INVENTORY_CAPACITY = 9;
inline constexpr uint8_t CHEST_INVENTORY_CAPACITY = 30;
inline constexpr uint16_t INVENTORY_STACK_LIMIT = 50;
const uint16_t PLAYER_INITIAL_HEALTH = 100;
const uint16_t PLAYER_INITIAL_ENERGY = 100;

const float TICK_MS = 1000.0f / 30.0f;
const int SPAWN_X = 1000;
const int SPAWN_Y = 1000;
const int PLAYER_STEP = 4;
