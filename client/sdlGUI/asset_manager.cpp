
#include "asset_manager.h"

#include <initializer_list>
#include <string>
#include <utility>
#include "dtos.h"

namespace Dimentions {
constexpr uint16_t FARMER_COLS = 6;
constexpr uint16_t FARMER_CELL_W = 16;
constexpr uint16_t FARMER_CELL_H = 32;
constexpr uint16_t ORE_SIZE = 16;
constexpr uint16_t SHEET_COLS = 24;
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
    auto add = [this](ResourceType t, ResourceState s, uint16_t idx) {
        ore_sprites.insert_or_assign(
            {t, s},
            Sprite{SpriteId::Ores, Dimentions::ORE_SIZE, Dimentions::ORE_SIZE,
                   static_cast<uint16_t>(idx / Dimentions::SHEET_COLS),
                   static_cast<uint16_t>(idx % Dimentions::SHEET_COLS)});
    };

    add(ResourceType::StoneRock,  ResourceState::Intact, 0);
    add(ResourceType::StoneRock,  ResourceState::Broken, 6);
    add(ResourceType::CoalRock,   ResourceState::Intact, 1);
    add(ResourceType::CoalRock,   ResourceState::Broken, 7);
    add(ResourceType::LapisRock,  ResourceState::Intact, 2);
    add(ResourceType::LapisRock,  ResourceState::Broken, 8);
    add(ResourceType::SilverRock, ResourceState::Intact, 3);
    add(ResourceType::SilverRock, ResourceState::Broken, 9);
    add(ResourceType::GoldRock,   ResourceState::Intact, 4);
    add(ResourceType::GoldRock,   ResourceState::Broken, 10);
}

void AssetManager::load_sprites_texture(SDL2pp::Renderer& renderer, const std::filesystem::path& assets_root) { 
    const auto characters = assets_root / "Characters";
    const auto ores = assets_root / "Caverns";

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
                    (index / Dimentions::FARMER_COLS), 
                    (index % Dimentions::FARMER_COLS)
                },
                flip
            };
            clip.sequence.push_back({frame, duration_ms});
        }

        ClipId id{SpriteId::FarmerBase, Action{state}, dir};
        clips.insert_or_assign(std::move(id), std::move(clip));
    };

    add(PlayerState::Idle, Direction::Down, {{0, 1000}}, false, true);
    add(PlayerState::Idle, Direction::Left, {{6, 1000}}, false, true);
    add(PlayerState::Idle, Direction::Right, {{6, 1000}}, false, true);
    add(PlayerState::Idle, Direction::Up, {{12, 1000}}, false, true);

    add(PlayerState::Walking, Direction::Down, {{0, 200}, {1, 200}, {0, 200}, {2, 200}}, true, true);
    add(PlayerState::Walking, Direction::Left, {{6, 200}, {7, 200}, {6, 200}, {8, 200}}, true, true);
    add(PlayerState::Walking, Direction::Right, {{6, 200}, {7, 200}, {6, 200}, {8, 200}}, true, true);
    add(PlayerState::Walking, Direction::Up, {{12, 200}, {13, 200}, {12, 200}, {14, 200}}, true, true);
}

SDL2pp::Texture& AssetManager::get_texture(SpriteId id) { return textures.at(id); }

const Clip& AssetManager::get_clip(const ClipId& id) const { return clips.at(id); }

const Sprite& AssetManager::get_sprite(ItemId id) const { return item_sprites.at(id); }

const Sound& AssetManager::get_sound(SoundId id) const { return sounds.at(id); }

const Sprite& AssetManager::get_sprite(ResourceType type, ResourceState state) const { return ore_sprites.at({type, state});}