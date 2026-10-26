#include <map>
#include "utils.h"

// TODO
class AssetManager{
private:
    // todos estos mapas se hardcodean en el constructor de assetmanager
    std::map<SpriteId, Sprite> sprites;
    std::map<ClipId, Clip> clips;
    std::map<SoundId, Sound> sounds;
public:
    // load completo y getters
};