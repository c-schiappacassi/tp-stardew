#include "common/foo.h"

#include <iostream>
#include <exception>
#include "game.h"

#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL.h>

using namespace SDL2pp;

int main(int argc, char* argv[]) try {
    if (argc < 4) { std::cerr << "Uso: " << argv[0] << " <host> <puerto> <nombre>\n"; return 1; }
    Game game(argv[1], argv[2], argv[3]);
    game.exe();
    return 0;
} catch (const std::exception& e) { std::cerr << e.what() << "\n"; return 1; }