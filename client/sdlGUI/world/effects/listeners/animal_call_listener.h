#pragma once
#include "position.h"
#include "protocol_types.h"

struct AnimalCall {
    AnimalType type;
    Position pos;
};

class AnimalCallListener {
public:
    virtual void on_call(const AnimalCall&) {}
    virtual ~AnimalCallListener() = default;
};
