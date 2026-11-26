#pragma once
#include <optional>

#include "../../utils.h"
#include "../entity_view.h"
#include "../../clip_player.h"
#include "../effects/visual/hit_flash.h"
#include "../effects/sounds/enemy_hurt_sound.h"
#include "../effects/sounds/skull_fire_sound.h"
#include "../effects/sounds/vampire_frenzy_sound.h"
#include "../effects/triggers/attack_trigger.h"
#include "../effects/triggers/frenzy_trigger.h"
#include "../effects/triggers/health_trigger.h"


#include "position.h"
#include "protocol_types.h"
#include "dtos.h"

class EnemyView: public EntityView {
private:
    Position pos{0, 0};
    EnemyType type;
       
    Direction facing = Direction::Down;
    EnemyState state;
    bool frenzied = false;
    uint16_t heath;
    SoundEmitter sfx;
    HealthTrigger health_trigger;
    HitFlash* hit_flash = nullptr;
    FrenzyTrigger frenzy_trigger;
    AttackTrigger attack_trigger;
    
    ClipPlayer clip_player;
public: 
    explicit EnemyView(const EnemySnapshot& data);
    void update(const EnemySnapshot& data);

    std::optional<float> depth(const RenderInfo& ctx) const override;
    void draw(const RenderInfo& ctx) override;
};