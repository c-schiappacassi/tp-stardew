#pragma once

#include <map>
#include <optional>

#include "crops/crop_manager.h"
#include "animals/animal_manager.h"
#include "decorations/decoration_manager.h"
#include "farm_config.h"
#include "spatial/farm_space.h"

class FarmManager {
private:
private:
    FarmConfig config;
    FarmSpace space;
    CropManager cropManager;
    AnimalManager animalManager;
    DecorationManager decorationManager;

public:
    explicit Farm(const FarmConfig& config = FarmConfig{});

    // Sin mapa, las posiciones representan celdas unitarias no negativas.
    // El mapa debe configurarse antes de tratar suelo o colocar entidades.
    bool setMap(const ZoneMap& map);
    const ZoneMap& getMap() const;
    bool canWalk(const Position& pos) const;
    bool tillSoil(const Position& pos);
    bool isTilled(const Position& pos) const;

    bool plant(EntityId id, const Position& pos, PlantType type);
    bool addAnimal(EntityId id, const Position& pos, AnimalType type);
    bool moveAnimal(EntityId id, const Position& pos);
    bool removeAnimal(EntityId id);
    bool placeDecoration(EntityId id, const Position& pos, ItemId item);
    std::optional<ItemId> removeDecoration(EntityId id);

    bool waterCrop(EntityId id);
    std::optional<FarmProduct> harvestCrop(EntityId id, double luck = 1.0);
    std::optional<FarmProduct> collectFromAnimal(EntityId id);
    void update();

    // Las consultas no permiten modificar entidades saltandose las reglas de Farm.
    const Crop* findCrop(EntityId id) const;
    const Animal* findAnimal(EntityId id) const;
    const Decoration* findDecoration(EntityId id) const;
    const std::map<EntityId, Crop>& getCrops() const;
    const std::map<EntityId, Animal>& getAnimals() const;
    const std::map<EntityId, Decoration>& getDecorations() const;
    const FarmConfig& getConfig() const;
};
