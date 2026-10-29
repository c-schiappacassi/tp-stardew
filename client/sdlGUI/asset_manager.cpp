#include "asset_manager.h"

namespace {
    constexpr int FARMER_COLS = 6;
    SDL2pp::Texture load_texture(SDL2pp::Renderer& r, const std::string& path) {
        SDL_Surface* raw = IMG_Load(path.c_str());
        if (!raw) {
            throw std::runtime_error("IMG_Load " + path + ": " + IMG_GetError());
        }
        return SDL2pp::Texture(r, SDL2pp::Surface(raw));
    }
}

void AssetManager::load_farmer_clips() {
    auto add = [this]( PlayerState state, Direction dir, std::initializer_list<std::pair<int, int>> frames, bool loop, bool mirror_left) {
        Clip clip;
        clip.loop = loop;
        const bool flip = mirror_left && dir == Direction::Left;
        for (const auto& [index, duration_ms] : frames) {
            clip.sequence.push_back({
                Frame{
                    index / FARMER_COLS,
                    index % FARMER_COLS,
                    flip
                },
                duration_ms
            });
        }
        ClipId id{
            SpriteId::FarmerBase,
            Action{state},
            dir,
            std::nullopt
        };
        clips.insert_or_assign(std::move(id), std::move(clip));
    };

    add(PlayerState::Idle, Direction::Down,{{0, 1000}}, false, true);
    add(PlayerState::Idle, Direction::Left,{{6, 1000}}, false, true);
    add(PlayerState::Idle, Direction::Right,{{6, 1000}}, false, true);
    add(PlayerState::Idle, Direction::Up,{{12, 1000}}, false, true);

    add(PlayerState::Walking, Direction::Down,{{0, 200}, {1, 200}, {0, 200}, {2, 200}}, true, true);
    add(PlayerState::Walking, Direction::Left, {{6, 200}, {7, 200}, {6, 200}, {8, 200}}, true, true);
    add(PlayerState::Walking, Direction::Right, {{6, 200}, {7, 200}, {6, 200}, {8, 200}},  true, true);
    add(PlayerState::Walking, Direction::Up, {{12, 200}, {13, 200}, {12, 200}, {14, 200}}, true, true);
}

void AssetManager::load_sprites(SDL2pp::Renderer& renderer) {
    sprites.emplace( SpriteId::FarmerBase,  Sprite{ load_texture(renderer, "common/assets/Sprites/Characters/Farmer Base.png"), 1, 2} );
    sprites.emplace( SpriteId::Hairstyles,  Sprite{ load_texture(renderer, "common/assets/Sprites/Characters/Hairstyles.png"), 1, 2} );
    sprites.emplace( SpriteId::Pants,  Sprite{ load_texture(renderer, "common/assets/Sprites/Characters/Pants.png"), 1, 2} );
    sprites.emplace( SpriteId::Shirts,  Sprite{ load_texture(renderer, "common/assets/Sprites/Characters/Shirts.png"), 1, 2} );
}