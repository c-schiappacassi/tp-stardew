#ifndef GAME_H
#define GAME_H

#include <string>

class Game {

    private:
        std::string id;
        std::string name;

    public:
        Game(const std::string& id, const std::string& name);

        const std::string& getId() const;
        const std::string& getName() const;
};

#endif