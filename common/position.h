#pragma once

// PODEMOS AGREGAR UN CONSTRUCTOR + SETTERS + GETTERS ASI NO INSTANCIAMOS UNA NUEVA POSICION TODO EL TIEMPO EN EL PROTOCOLO??
// (AHORA ESTOY HACIENDO ESO PERO ES CAMBIABLE Y ME PARECE MEJOR PRACTICA PODER IR MODIFICANDO LA POSICION PARA NO INSTANCIAR UNA NUEVA SIEMPRE PERO UDS DECIDEN)

class Position
{
private:
    int x;
    int y;
public:
    explicit Position(int x, int y) : x(x), y(y) {}

    int getX() const{
        return this->x;
    }
    
    int getY() const{
        return this->y;
    }
};