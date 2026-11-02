#pragma once
#include <cstdint>

enum class MessageType : uint8_t {

    CreateGame = 0x01,
    JoinGame = 0x02,
    ListGames = 0x03,  // lo usamos para cuando hay que retomar una partida
    ResumeGame = 0x04, // llamamos a todas las partidas y vemos si entre ellas
                       // esta la aprtida a la que nos queremos reunir
    ExitGame = 0x05,

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
    Ore,
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

// PLANTS
enum class PlantType : uint8_t {
    Strawberry = 0,
    Scallion = 1,
    Potato = 2,
    Pumpkin = 3
};

enum class CropState : uint8_t {
    Growing = 0,
    NeedsWater = 1,
    Ready = 2
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
    Cow = 0,
    Chicken = 1
};

enum class AnimalState : uint8_t {
    Idle = 0,
    Walking = 1,
    ReadyToMilk = 2
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

// CHEATS
enum class CheatType : uint8_t {
    SetLife = 1,
    AddGold = 2,
    RemoveGold = 3,
    CompleteSet = 4,
    NextLevel = 5,
};