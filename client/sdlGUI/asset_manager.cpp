
#include "asset_manager.h"

#include <initializer_list>
#include <string>
#include <utility>
#include "dtos.h"

namespace Dimentions {
    constexpr uint16_t FARMER_COLS = uint16_t(6);
    constexpr uint16_t FARMER_CELL_W = uint16_t(16);
    constexpr uint16_t FARMER_CELL_H = uint16_t(32);
    constexpr uint16_t COW_COLS = uint16_t(4);
    constexpr uint16_t COW_CELL_W = uint16_t(32);
    constexpr uint16_t COW_CELL_H = uint16_t(32);
    constexpr uint16_t CHICKEN_COLS = uint16_t(4);
    constexpr uint16_t CHICKEN_CELL_W = uint16_t(16);
    constexpr uint16_t CHICKEN_CELL_H = uint16_t(16);
    constexpr uint16_t ORE_SIZE = uint16_t(16);
    constexpr uint16_t CROP_CELL_W = uint16_t(16);
    constexpr uint16_t CROP_CELL_H = uint16_t(32);
}

SDL2pp::Texture AssetManager::load_texture(SDL2pp::Renderer& renderer, const std::filesystem::path& path) {
    SDL_Surface* raw = IMG_Load(path.string().c_str());
    if (!raw) {throw std::runtime_error("Could not load: " + path.string() + ": " + IMG_GetError());}
    return SDL2pp::Texture(renderer, SDL2pp::Surface(raw));
}

AssetManager::AssetManager(SDL2pp::Renderer& renderer, const std::filesystem::path& assets_root) {
    load_sprites_texture(renderer, assets_root / "Sprites" );
    load_farmer_clips();
    load_ore_sprites();
    load_item_sprites();
    load_crop_sprites();
    load_cow_clips();
}

void AssetManager::load_ore_sprites() {
    auto add = [this](ResourceType t, ResourceState s, uint16_t idx_row,  uint16_t idx_col) {
        ore_sprites.insert_or_assign(
            {t, s},
            Sprite{SpriteId::Ores, Dimentions::ORE_SIZE, Dimentions::ORE_SIZE, static_cast<uint16_t>(idx_row), static_cast<uint16_t>(idx_col)}
        );
    };

    add(ResourceType::StoneRock,  ResourceState::Intact, 1,14);
    add(ResourceType::StoneRock,  ResourceState::Broken, 1,15);
    add(ResourceType::CoalRock,   ResourceState::Intact, 1,0);
    add(ResourceType::CoalRock,   ResourceState::Broken, 1,1);
    add(ResourceType::LapisRock, ResourceState::Intact, 0,14);
    add(ResourceType::LapisRock, ResourceState::Broken, 0,13);
    add(ResourceType::SilverRock,  ResourceState::Intact, 0,12);
    add(ResourceType::SilverRock,  ResourceState::Broken, 0,13);
    add(ResourceType::GoldRock,   ResourceState::Intact, 0,8);
    add(ResourceType::GoldRock,   ResourceState::Broken, 0,9);
}

void AssetManager::load_crop_sprites() {
    auto add = [this](CropType t, CropState s, uint16_t idx_row,  uint16_t idx_col) {
        crop_sprites.insert_or_assign(
            {t, s},
            Sprite{SpriteId::Crops, Dimentions::CROP_CELL_W, Dimentions::CROP_CELL_H, static_cast<uint16_t>(idx_row), static_cast<uint16_t>(idx_col)}
        );
    };

    add(CropType::Potato,  CropState::JustPlanted, 29, 160);
    add(CropType::Potato,  CropState::Sprouting, 29, 176);
    add(CropType::Potato,  CropState::Growing, 29, 208);
    add(CropType::Potato,  CropState::Ready, 29, 224);

    add(CropType::Pumpkin,  CropState::JustPlanted, 285, 0);
    add(CropType::Pumpkin,  CropState::Sprouting, 285, 48);
    add(CropType::Pumpkin,  CropState::Growing, 285, 80);
    add(CropType::Pumpkin,  CropState::Ready, 285, 285);

    add(CropType::Strawberry,  CropState::JustPlanted, 318, 144);
    add(CropType::Strawberry,  CropState::Sprouting, 318, 192);
    add(CropType::Strawberry,  CropState::Growing, 318, 240);
    add(CropType::Strawberry,  CropState::Ready, 318, 224);
    
    add(CropType::Scallion,  CropState::JustPlanted, 61, 16);
    add(CropType::Scallion,  CropState::Sprouting, 61, 48);
    add(CropType::Scallion,  CropState::Growing, 61, 64);
    add(CropType::Scallion,  CropState::Ready, 61, 80);
}

void AssetManager::load_item_sprites() {
    item_sprites.insert_or_assign( ItemId::Pickaxe , Sprite{ SpriteId::Tools , 16, 16 , 97,304} );
    item_sprites.insert_or_assign( ItemId::Sword , Sprite{ SpriteId::Weapons , 16, 16 , 16, 48} );
    item_sprites.insert_or_assign( ItemId::WateringCan , Sprite{ SpriteId::Tools , 16, 11 , 228 , 32} );
    item_sprites.insert_or_assign( ItemId::Shovel , Sprite{ SpriteId::Tools , 16, 16 , 33, 304} );

    item_sprites.insert_or_assign( ItemId::Coal , Sprite{ SpriteId::Ores , 16, 16 , 108, 16} );
    item_sprites.insert_or_assign( ItemId::Lapis , Sprite{ SpriteId::Ores , 16, 16 , 108, 64} );
    item_sprites.insert_or_assign( ItemId::Silver , Sprite{ SpriteId::Ores , 15, 15 , 68, 145} );
    item_sprites.insert_or_assign( ItemId::Gold , Sprite{ SpriteId::Ores , 16, 16 , 108, 94} );
}

void AssetManager::load_sprites_texture(SDL2pp::Renderer& renderer, const std::filesystem::path& assets_root) { 
    const auto characters = assets_root / "Characters";
    const auto ores = assets_root / "Caverns";
    const auto menus = assets_root / "Menus";
    const auto farm = assets_root / "Farm";

    textures.emplace(SpriteId::MenuTiles, load_texture(renderer, menus / "MenuTiles..png"));
    textures.emplace(SpriteId::FarmerBase, load_texture(renderer, characters / "Farmer Base.png"));
    textures.emplace(SpriteId::Hairstyles, load_texture(renderer, characters / "Hairstyles.png"));
    textures.emplace(SpriteId::Pants, load_texture(renderer, characters / "Pants.png"));
    textures.emplace(SpriteId::Shirts, load_texture(renderer, characters / "Shirts.png"));
    textures.emplace(SpriteId::Tools, load_texture(renderer, characters / "Tools.png"));
    textures.emplace(SpriteId::Weapons, load_texture(renderer, characters / "Weapons.png"));
    textures.emplace(SpriteId::Ores, load_texture(renderer, ores / "Mines.png"));
    textures.emplace(SpriteId::Crops, load_texture(renderer, farm / "Crops" / "Crops.png"));
    textures.emplace(SpriteId::BronwCow, load_texture(renderer, farm / "Animals" / "Cow (Brown).png"));
    textures.emplace(SpriteId::BronwChicken, load_texture(renderer, farm / "Animals" / "Chicken (Brown).png"));
}

void AssetManager::load_farmer_clips() {
    auto add = [this](PlayerState state, Direction dir,  std::initializer_list<std::pair<int, int>> frames, bool loop, bool mirror_left) {
        Clip clip;
        clip.loop = loop;
        const bool flip = mirror_left && dir == Direction::Left;

        for (const auto& [index, duration_ms]: frames) {
            AnimatedFrame frame{
                Sprite{
                    SpriteId::FarmerBase, 
                    Dimentions::FARMER_CELL_W, Dimentions::FARMER_CELL_H, 
                    static_cast<uint16_t>(index / Dimentions::FARMER_COLS), 
                    static_cast<uint16_t>(index % Dimentions::FARMER_COLS)
                },
                flip
            };
            clip.sequence.push_back({frame, duration_ms});
        }

        ClipId id{SpriteId::FarmerBase, Action{state}, dir};
        clips.insert_or_assign(std::move(id), std::move(clip));
    };

    add(PlayerState::Idle, Direction::Down, {{0, 200}}, false, true);
    add(PlayerState::Idle, Direction::Left, {{6, 200}}, false, true);
    add(PlayerState::Idle, Direction::Right, {{6, 200}}, false, true);
    add(PlayerState::Idle, Direction::Up, {{12, 200}}, false, true);

    add(PlayerState::Walking, Direction::Down, {{0, 200}, {1, 200}, {0, 200}, {2, 200}}, true, true);
    add(PlayerState::Walking, Direction::Left, {{6, 200}, {7, 200}, {6, 200}, {8, 200}}, true, true);
    add(PlayerState::Walking, Direction::Right, {{6, 200}, {7, 200}, {6, 200}, {8, 200}}, true, true);
    add(PlayerState::Walking, Direction::Up, {{12, 200}, {13, 200}, {12, 200}, {14, 200}}, true, true);

    add(PlayerState::UsingTool, Direction::Down,  {{66, 100}, {67, 100}, {68, 100}, {69, 100}, {70, 100}}, false, true);
    add(PlayerState::UsingTool, Direction::Right, {{48, 100}, {49, 100}, {50, 100}, {51, 100}, {52, 100}}, false, true);
    add(PlayerState::UsingTool, Direction::Left,  {{48, 100}, {49, 100}, {50, 100}, {51, 100}, {52, 100}}, false, true);
    add(PlayerState::UsingTool, Direction::Up,    {{36, 100}, {37, 100}, {38, 100}, {39, 100}, {40, 100}}, false, true);
}


void AssetManager::load_cow_clips() {
    auto add = [this](AnimalState state, Direction dir, std::initializer_list<std::pair<int, int>> frames, bool loop, bool mirror_left) {
        Clip clip;
        clip.loop = loop;
        const bool flip = mirror_left && dir == Direction::Left;

        for (const auto& [index, duration_ms] : frames) {
            AnimatedFrame frame{
                Sprite{
                    SpriteId::BronwCow,
                    Dimentions::COW_CELL_W, Dimentions::COW_CELL_H,
                    static_cast<uint16_t>(index / Dimentions::COW_COLS),
                    static_cast<uint16_t>(index % Dimentions::COW_COLS)
                },
                flip
            };
            clip.sequence.push_back({frame, duration_ms});
        }

        ClipId id{SpriteId::BronwCow, Action{state}, dir};
        clips.insert_or_assign(std::move(id), std::move(clip));
    };

    constexpr int IDLE_MS = 400;
    constexpr int WALK_MS = 150;

    add(AnimalState::Idle, Direction::Down,  {{0, IDLE_MS},  {1, IDLE_MS}},  true, false);
    add(AnimalState::Idle, Direction::Right, {{4, IDLE_MS},  {5, IDLE_MS}},  true, false);
    add(AnimalState::Idle, Direction::Up,    {{8, IDLE_MS},  {9, IDLE_MS}},  true, false);
    add(AnimalState::Idle, Direction::Left,  {{12, IDLE_MS}, {13, IDLE_MS}}, true, false);

    add(AnimalState::Walking, Direction::Down,  {{0, WALK_MS},  {1, WALK_MS},  {2, WALK_MS},  {3, WALK_MS}},  true, false);
    add(AnimalState::Walking, Direction::Right, {{4, WALK_MS},  {5, WALK_MS},  {6, WALK_MS},  {7, WALK_MS}},  true, false);
    add(AnimalState::Walking, Direction::Up,    {{8, WALK_MS},  {9, WALK_MS},  {10, WALK_MS}, {11, WALK_MS}}, true, false);
    add(AnimalState::Walking, Direction::Left,  {{12, WALK_MS}, {13, WALK_MS}, {14, WALK_MS}, {15, WALK_MS}}, true, false);
}

void AssetManager::load_chicken_clips() {
    auto add = [this](AnimalState state, Direction dir, std::initializer_list<std::pair<int, int>> frames, bool loop, bool mirror_left) {
        Clip clip;
        clip.loop = loop;
        const bool flip = mirror_left && dir == Direction::Left;

        for (const auto& [index, duration_ms] : frames) {
            AnimatedFrame frame{
                Sprite{
                    SpriteId::BronwChicken,
                    Dimentions::CHICKEN_CELL_W, Dimentions::CHICKEN_CELL_H,
                    static_cast<uint16_t>(index / Dimentions::CHICKEN_COLS),
                    static_cast<uint16_t>(index % Dimentions::CHICKEN_COLS)
                },
                flip
            };
            clip.sequence.push_back({frame, duration_ms});
        }

        ClipId id{SpriteId::BronwChicken, Action{state}, dir};
        clips.insert_or_assign(std::move(id), std::move(clip));
    };

    constexpr int IDLE_MS = 400;
    constexpr int WALK_MS = 120;

    constexpr int LEFT_ROW  = 0;
    constexpr int RIGHT_ROW = 1 *  Dimentions::CHICKEN_COLS;
    constexpr int FRONT_ROW = 6 *  Dimentions::CHICKEN_COLS;
    constexpr int BACK_ROW  = FRONT_ROW;

    add(AnimalState::Idle, Direction::Left,  {{LEFT_ROW,  IDLE_MS}, {LEFT_ROW + 1,  IDLE_MS}}, true, false);
    add(AnimalState::Idle, Direction::Right, {{RIGHT_ROW, IDLE_MS}, {RIGHT_ROW + 1, IDLE_MS}}, true, false);
    add(AnimalState::Idle, Direction::Down,  {{FRONT_ROW, IDLE_MS}, {FRONT_ROW + 1, IDLE_MS}}, true, false);
    add(AnimalState::Idle, Direction::Up,    {{BACK_ROW,  IDLE_MS}, {BACK_ROW + 1,  IDLE_MS}}, true, false);

    add(AnimalState::Walking, Direction::Left, {{LEFT_ROW, WALK_MS}, {LEFT_ROW + 1, WALK_MS}, {LEFT_ROW + 2, WALK_MS}, {LEFT_ROW + 3, WALK_MS}}, true, false);
    add(AnimalState::Walking, Direction::Right, {{RIGHT_ROW, WALK_MS}, {RIGHT_ROW + 1, WALK_MS}, {RIGHT_ROW + 2, WALK_MS}, {RIGHT_ROW + 3, WALK_MS}}, true, false);
    add(AnimalState::Walking, Direction::Down, {{FRONT_ROW, WALK_MS}, {FRONT_ROW + 1, WALK_MS}, {FRONT_ROW + 2, WALK_MS}, {FRONT_ROW + 3, WALK_MS}}, true, false);
    add(AnimalState::Walking, Direction::Up, {{BACK_ROW, WALK_MS}, {BACK_ROW + 1, WALK_MS}, {BACK_ROW + 2, WALK_MS}, {BACK_ROW + 3, WALK_MS}}, true, false);
}


SDL2pp::Texture& AssetManager::get_texture(SpriteId id) { return textures.at(id); }

const Clip& AssetManager::get_clip(const ClipId& id) const { return clips.at(id); }

const Sprite& AssetManager::get_sprite(ItemId id) const { return item_sprites.at(id); }

const Sprite& AssetManager::get_sprite(CropType type, CropState state) const { return crop_sprites.at({type, state}); }

const Sound& AssetManager::get_sound(SoundId id) const { return sounds.at(id); }

const Sprite& AssetManager::get_sprite(ResourceType type, ResourceState state) const { return ore_sprites.at({type, state});}

const Sprite* AssetManager::find_sprite(ItemId id) const {
    const auto it = item_sprites.find(id);
    return it == item_sprites.end() ? nullptr : &it->second;
}