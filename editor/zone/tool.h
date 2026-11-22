#ifndef TOOL_H
#define TOOL_H

#include "../model/catalog.h"

struct Tool {
    enum class Kind { PaintFloor, PlaceElement, Move, Erase };

    Kind kind;
    TileTextureId floor;
    ElementId element;
};

#endif  // TOOL_H