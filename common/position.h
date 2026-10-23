#pragma once


class Position
{
private:
    int x;
    int y;
public:
    explicit Position(int x, int y) : x(x), y(y) {};

    int getX() const{
        return this->x;
    }
    
    int getY() const{
        return this->y;
    }
};