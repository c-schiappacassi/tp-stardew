#pragma once

class DungeonManager;
class FarmManager;
class InteriorRooms;
class EntityIdGenerator;
struct PersistenceDTO;

class WorldPersistence {
private:
    DungeonManager& dungeon;
    FarmManager& farm;
    InteriorRooms& interiors;
    EntityIdGenerator& entityIds;

public:
    WorldPersistence(DungeonManager& dungeon, FarmManager& farm, InteriorRooms& interiors, EntityIdGenerator& ids);
    void restoreFarm(const PersistenceDTO& saved);
    void saveFarmAndStorage(PersistenceDTO& saved) const;
};
