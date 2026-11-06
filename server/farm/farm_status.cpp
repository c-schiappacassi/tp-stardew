#include "farm_status.h"
#include "farm_manager.h"

FarmData FarmStatus::build(const FarmManager& farm) {
    FarmData result;
    for (const auto& [id, crop] : farm.getCrops()) {
        result.crops.push_back(crop.getPublicData());
    }
    for (const auto* animal : farm.getAnimals()) {
        result.animals.push_back(animal->getPublicData());
    }
    for (const auto& [id, decoration] : farm.getDecorations()) {
        const DecorationData data{id, decoration.getPosition(), decoration.getItem()};
        result.decorations.push_back(data);
    }
    result.tilledSoil = farm.getTilledPositions();
    return result;
}
