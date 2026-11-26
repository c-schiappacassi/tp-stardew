#pragma once

#include "position.h"
#include "protocol_types.h"

struct CropWatering {
    CropType type;
    Position pos;
};

class CropListener {
public:
    virtual void on_watered(const CropWatering&) {}
    virtual ~CropListener() = default;
};
