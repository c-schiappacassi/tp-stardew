#pragma once

#include "dtos.h"

class FarmManager;

class FarmStatus {
public:
    static FarmData build(const FarmManager& farm);
};
