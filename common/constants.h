#pragma once

const int SUCCESS = 0;
const int FAILURE = 1;

const unsigned int INITIAL_PLAYERS = 1;
const char SERVER_QUIT_CHAR = 'q';

const char* const CMD_LIST = "listar";
const char* const CMD_CREATE = "crear";
const char* const CMD_JOIN = "unirse";
const char* const CMD_END = "fin";

const char* const CREATE_SUCCESS_MSG = "Creacion exitosa";
const char* const CREATE_FAILURE_MSG = "Creacion fallida";
const char* const JOIN_SUCCESS_MSG = "Union exitosa";
const char* const JOIN_FAILURE_MSG = "Union fallida";

const char* const GAME_FULL_PREFIX = "Partida llena ";
const char* const GAME_FULL_SUFFIX = "...";

const char* const LIST_SEPARATOR = "/";
const char* const SPACE_SEPARATOR = " ";
const char* const NEXT_LINE = "\n";

const char* const UNEXPECTED_ERROR_MSG = "Unexpected error: ";
const char* const UNKNOWN_ERROR_MSG = "Error desconocido";
const char* const SERVER_ERROR_MSG = "Error de conexion con el servidor: ";
const char* const SOCKET_ERROR_MSG = "Socket error: ";
const char* const UNKNOWN_OPERATOR_MSG = "Operacion desconocida";

const int FPS = 30;
