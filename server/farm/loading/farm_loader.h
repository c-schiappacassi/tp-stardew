#pragma once

#include "farm_layout.h"

class FarmManager;
class EntityIdGenerator;

class FarmLoader {
public:
    bool load(const FarmLayout& layout, FarmManager& farm, EntityIdGenerator& ids) const;
};
