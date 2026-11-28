#include "zone_options.h"

ZoneOptions farmOptions() {
    return {
        {TileTextureId::Grass, TileTextureId::Dirt, TileTextureId::StonePath, TileTextureId::ConnctionStonePathGrassCorner,
         TileTextureId::ConnctionStonePathGrassTop, TileTextureId::ConnctionStonePathGrassTopRightCorner,
         TileTextureId::ConnctionStonePathGrassLeftSide, TileTextureId::ConnctionStonePathGrassRightSide,
         TileTextureId::ConnctionStonePathGrassBottomLeftCorner, TileTextureId::ConnctionStonePathGrassBottom,
         TileTextureId::ConnctionStonePathGrassBottomRightCorner},
        {
            buildingElement(BuildingId::FarmHouse),
            buildingElement(BuildingId::StoneCabin),
            buildingElement(BuildingId::WoodCabin),
            buildingElement(BuildingId::BrickCabin),

            decorElement(DecorId::MapleTree),
            decorElement(DecorId::PineTree),
            decorElement(DecorId::PinkTree),
            decorElement(DecorId::YellowTree),
            decorElement(DecorId::BlueTree),
            decorElement(DecorId::MushroomTree),
            decorElement(DecorId::SimpleBush),
            decorElement(DecorId::RedBBerryBush),
            decorElement(DecorId::PinkBush),
            decorElement(DecorId::PurpleBerryBush),
            decorElement(DecorId::BlueBush),
            decorElement(DecorId::BigRedBush),
            decorElement(DecorId::PlainGrass),
            decorElement(DecorId::PinkFlowerGrass),
            decorElement(DecorId::OrangeFlowerGrass),
            decorElement(DecorId::OrangeGrass),
            decorElement(DecorId::VelvetGrass),
            decorElement(DecorId::BlueGrass),
            decorElement(DecorId::RedGrass),
            decorElement(DecorId::PinkFlower),
            decorElement(DecorId::BlueFlower),
            decorElement(DecorId::BabyRedMushrom),
            decorElement(DecorId::BigRedMushroom),
            decorElement(DecorId::FenceFront),
            decorElement(DecorId::FenceBack),
            decorElement(DecorId::FenceSideLeft),
            decorElement(DecorId::FenceSideRight),

            portalElement(PortalId::FarmToMine),
        },
    };
}

ZoneOptions townOptions() {
    return {
        {TileTextureId::Grass, TileTextureId::ConnctionStonePathGrassCorner,
         TileTextureId::ConnctionStonePathGrassLeftSide, TileTextureId::ConnctionStonePathGrassTop,
         TileTextureId::ConnctionStonePathGrassBottom, TileTextureId::ConnctionStonePathGrassTopRightCorner,
         TileTextureId::ConnctionStonePathGrassBottomRightCorner, TileTextureId::ConnctionStonePathGrassBottomLeftCorner,
         TileTextureId::ConnctionStonePathGrassRightSide, TileTextureId::StonePath},
        {
            buildingElement(BuildingId::PierresShop),
            buildingElement(BuildingId::ClintsShop),
            buildingElement(BuildingId::MarniesShop),
            buildingElement(BuildingId::RobinsShop),
            buildingElement(BuildingId::ComunityCenter),

            decorElement(DecorId::MapleTree),
            decorElement(DecorId::PineTree),
            decorElement(DecorId::PinkTree),
            decorElement(DecorId::YellowTree),
            decorElement(DecorId::BlueTree),
            decorElement(DecorId::SimpleBush),
            decorElement(DecorId::RedBBerryBush),
            decorElement(DecorId::PinkBush),
            decorElement(DecorId::PurpleBerryBush),
            decorElement(DecorId::BlueBush),
            decorElement(DecorId::BigRedBush),
            decorElement(DecorId::PlainGrass),
            decorElement(DecorId::PinkFlowerGrass),
            decorElement(DecorId::OrangeFlowerGrass),
            decorElement(DecorId::PinkFlower),
            decorElement(DecorId::BlueFlower),
            decorElement(DecorId::SmallBlueFlower),
            decorElement(DecorId::StreatLamp),
            decorElement(DecorId::FenceFront),
            decorElement(DecorId::FenceBack),
            decorElement(DecorId::FenceSideLeft),
            decorElement(DecorId::FenceSideRight),
            decorElement(DecorId::PottedPlants),
            decorElement(DecorId::SmallBench),
            decorElement(DecorId::Table),
            decorElement(DecorId::ChairFacingRight),
            decorElement(DecorId::ChairFacingLeft),
            decorElement(DecorId::PottedFern),
            decorElement(DecorId::PottedFlower),
            decorElement(DecorId::FlowerBox),
            decorElement(DecorId::Mailbox),
            decorElement(DecorId::TrashCan),
        },
    };
}

ZoneOptions mineOptions() {
    return {
        {TileTextureId::DungeonFloor, TileTextureId::DungeonDottedFloor, TileTextureId::DungeonPebbledFloor},
        {
            decorElement(DecorId::WeirdGreenPlant),
            decorElement(DecorId::SmallFern),
            decorElement(DecorId::BigFern),
            decorElement(DecorId::PinkMushrom),
            decorElement(DecorId::YellowMushrom),
            decorElement(DecorId::BabyRedMushrom),
            decorElement(DecorId::BigRedMushroom),
            decorElement(DecorId::WallSpiderWeb),
            decorElement(DecorId::WallLantern),
            decorElement(DecorId::SmallWallLantern),

            portalElement(PortalId::MineToFarm),
            portalElement(PortalId::ToNextLevel),
        },
    };
}