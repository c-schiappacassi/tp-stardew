#pragma once
#include <map>
#include "protocol_types.h"
#include "utils.h"


// TODO
class AssetManager {
private:
    std::map<SpriteId, Sprite> sprites;
    std::map<ClipId, Clip> clips;
    std::map<SoundId, Sound> sounds;
public:
    const Sprite& get_sprite(SpriteId id);
    const Sound& get_sound(SoundId id);
};