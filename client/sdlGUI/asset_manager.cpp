
#include "asset_manager.h"

#include <initializer_list>
#include <string>
#include <utility>
#include "dtos.h"

namespace Dimentions {
constexpr uint16_t FARMER_COLS = uint16_t(6);
constexpr uint16_t FARMER_CELL_W = uint16_t(16);
constexpr uint16_t FARMER_CELL_H = uint16_t(32);
constexpr uint16_t ORE_SIZE = uint16_t(16);
constexpr uint16_t ORE_SHEET_COLS = uint16_t(24);
}

SDL2pp::Texture AssetManager::load_texture(SDL2pp::Renderer& renderer, const std::filesystem::path& path) {
    SDL_Surface* raw = IMG_Load(path.string().c_str());

    if (!raw) {
        throw std::runtime_error("Could not load: " + path.string() + ": " + IMG_GetError());
    }

    return SDL2pp::Texture(renderer, SDL2pp::Surface(raw));
}

AssetManager::AssetManager(SDL2pp::Renderer& renderer, const std::filesystem::path& assets_root) {
    load_sprites_texture(renderer, assets_root / "Sprites" );
    load_farmer_clips();
    load_ore_sprites();
}

void AssetManager::load_ore_sprites() {
    auto add = [this](ResourceType t, ResourceState s, uint16_t idx_row,  uint16_t idx_col) {
        ore_sprites.insert_or_assign(
            {t, s},
            Sprite{SpriteId::Ores, Dimentions::ORE_SIZE, Dimentions::ORE_SIZE,
                   static_cast<uint16_t>(idx_row / Dimentions::ORE_SHEET_COLS),
                   static_cast<uint16_t>(idx_col % Dimentions::ORE_SHEET_COLS)});
    };

    add(ResourceType::StoneRock,  ResourceState::Intact, 1,0);
    add(ResourceType::StoneRock,  ResourceState::Broken, 1,0);
    add(ResourceType::CoalRock,   ResourceState::Intact, 1,0);
    add(ResourceType::CoalRock,   ResourceState::Broken, 1,0);
    add(ResourceType::LapisRock,  ResourceState::Intact, 1,0);
    add(ResourceType::LapisRock,  ResourceState::Broken, 1,0);
    add(ResourceType::SilverRock, ResourceState::Intact, 1,0);
    add(ResourceType::SilverRock, ResourceState::Broken, 1,0);
    add(ResourceType::GoldRock,   ResourceState::Intact, 1,0);
    add(ResourceType::GoldRock,   ResourceState::Broken, 1,0);
}

void AssetManager::load_sprites_texture(SDL2pp::Renderer& renderer, const std::filesystem::path& assets_root) { 
    const auto characters = assets_root / "Characters";
    const auto ores = assets_root / "Caverns";
    const auto menus = assets_root / "Menus";

    textures.emplace(SpriteId::MenuTiles, load_texture(renderer, menus / "MenuTiles..png"));
    textures.emplace(SpriteId::FarmerBase, load_texture(renderer, characters / "Farmer Base.png"));
    textures.emplace(SpriteId::Hairstyles, load_texture(renderer, characters / "Hairstyles.png"));
    textures.emplace(SpriteId::Pants, load_texture(renderer, characters / "Pants.png"));
    textures.emplace(SpriteId::Shirts, load_texture(renderer, characters / "Shirts.png"));
    textures.emplace(SpriteId::Ores, load_texture(renderer, ores / "Mines.png"));
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

SDL2pp::Texture& AssetManager::get_texture(SpriteId id) { return textures.at(id); }

const Clip& AssetManager::get_clip(const ClipId& id) const { return clips.at(id); }

const Sprite& AssetManager::get_sprite(ItemId id) const { return item_sprites.at(id); }

const Sound& AssetManager::get_sound(SoundId id) const { return sounds.at(id); }

const Sprite& AssetManager::get_sprite(ResourceType type, ResourceState state) const { return ore_sprites.at({type, state});}

const Sprite* AssetManager::find_sprite(ItemId id) const {
    const auto it = item_sprites.find(id);
    return it == item_sprites.end() ? nullptr : &it->second;
}