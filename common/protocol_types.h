#pragma once
#include <cstdint>

enum class MessageType : uint8_t {

    CreateGame = 0x01,
    JoinGame = 0x02,
    ListGames = 0x03,  // lo usamos para cuando hay que retomar una partida
    ResumeGame = 0x04, // llamamos a todas las partidas y vemos si entre ellas
                        // esta la aprtida a la que nos queremos reunir
    CheckCode = 0x05,
    ExitGame = 0x06,

    Move = 0x10,
    SelectItem = 0x11,   // cofre a inventario
    UseItem = 0x12,
    Interact = 0x13,
    Chat = 0x14,
    GiftResponse = 0x15,
    DayOk = 0x16,
    Donate = 0x17,       // para completar las misiones
    ItemTransfer = 0x18, // inventario a cofre
    SelectSlot = 0x19,

    Status = 0x80,
    GameList = 0x81,
    GameJoined = 0x82,
    MultiplayerCode = 0x83,
    ChatMessage = 0x84,
    GiftOffer = 0x85,
    DayEnd = 0x86,
    GameCompleted = 0x87,
    CommunityState = 0x88, // avance de las misiones
};

enum class StatusCode : uint8_t { 
    Ok = 0, 
    Error = 1 
};

enum class Direction : uint8_t {
    None = 0,
    Up = 1,
    Down = 2,
    Left = 3,
    Right = 4
};

enum class Zone : uint8_t {
    Farm = 0,
    Town = 1,
    Dungeon = 2
};

enum class TargetType : uint8_t {
    Npc = 0,
    Player = 1,
    Animal = 2,
    Chest = 3,
    Bed = 4,
    Ore = 5,
    DungeonPassage = 6
};

enum class PlayerState : uint8_t {
    Idle = 0,
    Walking = 1,
    UsingTool = 2,
    Unconscious = 3,
    Sleeping = 4
};

// ENEMIES
enum class EnemyType : uint8_t {
    Skeleton = 0,
    Orc = 1,
    VampireMale = 2,
    VampireFemale = 3,
    Skull = 4,
    Traps = 5,
};

enum class EnemyState : uint8_t {
    Idle = 0, // hablar como va a ser la visual, los ves y ya estan moviendose, nunca paran salvo cuando los matas un segundo (efecto dramatico) y despues a dead
    Moving = 1,
    Attacking = 2,
    Dead = 3,
};

enum class FlameType : uint8_t { Blue = 0, Green = 1, Common };
enum class TrapType : uint8_t { Spikes = 0, WallFlame = 1 };
enum class TrapState : uint8_t { Inactive = 0, Warning = 1, Active = 2 };

// PLANTS
enum class CropType : uint8_t {
    Strawberry = 0,
    Scallion = 1,
    Potato = 2,
    Pumpkin = 3
};

enum class CropState : uint8_t {
    JustPlanted,
    Sprouting,
    Growing,
    Ready
};

enum class Quality : uint8_t {
    None = 0,
    Base = 1,       // x1
    Good = 2,       // x1.25
    Excellent = 3,  // x2
    Incredible = 5, // x4
    Perfect = 6     // x8
};

// ANIMALS
enum class AnimalType : uint8_t {
    BrownCow,
    WhiteCow,
    BrownChicken,
    WhiteChicken,
    BlueChicken,
    VoidChicken,
};

enum class AnimalState : uint8_t {
    Idle = 0,
    Walking = 1,
};

// ROCKS
enum class ResourceType : uint8_t {
    StoneRock,
    CoalRock,
    LapisRock,
    SilverRock,
    GoldRock
};

enum class ResourceState : uint8_t {
    Intact = 0,
    Broken = 1,
};

enum class MineralState : uint8_t {
    Intact = 0,
    Broken = 1
};

// ITEMS
enum class ItemId : uint16_t {
    Pickaxe = 1,
    WateringCan = 2,
    Shovel = 3,
    Sword = 4,

    StrawberrySeed = 10,
    ScallionSeed = 11,
    PotatoSeed = 12,
    PumpkinSeed = 13,

    Strawberry = 20, // usamos cultivo + calidad
    Scallion = 21,
    Potato = 22,
    Pumpkin = 23,

    Coal = 30,
    Lapis = 31,
    Silver = 32,
    Gold = 33,

    Milk = 40,
    Egg = 41,

    // podemos agregar mas cosas al shop
    Chest = 50,
    Table = 51,
    Chair = 52,
    Lamp = 53,
    Rug = 54,
    Plant = 55,
    Painting = 56
};

enum class EmoteId : uint16_t {
    Alert
};

// CHEATS
enum class CheatType : uint8_t {
    SetLife = 1,
    AddGold = 2,
    RemoveGold = 3,
    CompleteSet = 4,
    NextLevel = 5,
};

// TODO: completar
enum class TileTextureId : uint8_t{
    Grass,
    Dirt,
    StoneCliftTop,
    StoneCliftSide,
    StoneCliftTopRightCorner,
    StoneCliftBottom,
    StoneCliftBottomRigthCorner,
    DungeonWall,
    DungeonWallCorner,
    StonePath,
    ConnctionStonePathGrassCorner,
    ConnctionStonePathGrassLeftSide,
    ConnctionStonePathGrassTop,
    ConnctionStonePathGrassBottom,
    WateredDirt,
    TilledDirt,
    DungeonFloor,
    DungeonVoid,
    DungeonLeftWall,
    DungeonRightWall,
    DungeonBottom,
    DungeonBottomLeftCorner1,
    DungeonBottomRightCorner1,
    DungeonInnerCornerTopRight,
    DungeonInnerCornerTopLeft,
};

enum class FramePieceId : uint8_t{
    StoneCliftTop,
    StoneCliftTopLeftCorner,
    StoneCliftDropRight,
    StoneCliftLeftSide,
    StoneCliftRightSide,
    StoneCliftBottom,
    StoneCliftTopRightCorner,
    StoneCliftDropLeft,
    StoneCliftBottomLeftCorner,
    StoneCliftBottomRightCorner,
    DungeonTopWall,
    DungeonTopLeftCorner1,
    DungeonTopLeftCorner2,
    DungeonTopRightCorner1,
    DungeonTopRightCorner2,
    DungeonBottomLeftCorner2,
    DungeonBottomRightCorner2,
    DungeonLantern,
    DungeonTopWallEndRight,
    DungeonTopWallEndLeft,
};

enum class DecorId : uint8_t{
    MapleTree,
    PineTree,
    MushroomTree,
    PinkTree,
    YellowTree,
    BlueTree,
    PinkFlowerGrass,
    OrangeFlowerGrass,
    OrangeGrass,
    VelvetGrass,
    BlueGrass,
    PlainGrass,
    RedGrass,
    SimpleBush,
    RedBBerryBush,
    PinkBush,
    PurpleBerryBush,
    BlueBush,
    BigRedBush,
    StreatLamp,
    FenceFront,
    FenceBack,
    PinkFlower,
    BlueFlower,
    WeirdGreenPlant,
    SmallFern,
    BigFern,
    PinkMushrom,
    YellowMushrom,
    BabyRedMushrom,
    BigRedMushroom,
    DungeonPilar,
    DungeonTorch,
    DungeonChain,
    DungeonWindow,
    DungeonWallCrack,
    SpiderWeb,
    CornerSpiderWeb,
    PottedPlants,
    SmallBlueFlower,
};


enum class PortalId : uint8_t{
    FarmToMine,
    ToNextLevel, // en todos los niveles excepto el que quedo ultimo
    MineToFarm, // en todos los niveles
};


enum class FurnitureId : uint8_t{
    Chair,
    Table,
    Bed,
    BedSheet,
    Bonsai,
    PottedTree,
    BlueArmCouch,
    Tv,
    BeigeRug,
    ClockFirePlace,
    BlueRug,
    CatFirePlace,
    SimpleFirePlace,
    BlueJunimoPushee,
    GreenJunimoPushee,
    YellowJunimoPushee,
    PinkJunimoPushee,
    SimpleLamp,
    BookShelf,
    SquareLamp,
    RoundTable,
    GreenStool,
    BrownSofa,
    BrownArmCouch,
};

enum class BuildingId : uint8_t{
    FarmHouse,
    StoneCabin,
    WoodCabin,
    BrickCabin,
    MarniesShop,
    PierresShop,
    RobinsShop,
    ClintsShop,
    ComunityCenter,
};