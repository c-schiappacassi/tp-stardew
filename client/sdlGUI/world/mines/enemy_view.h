#pragma once
#include <optional>

#include "../../utils.h"
#include "../entity_view.h"
#include "../../clip_player.h"
#include "../effects/health_trigger.h"
#include "../effects/hit_flash.h"

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
    HealthTrigger health_trigger;
    HitFlash* hit_flash = nullptr;
    
    ClipPlayer clip_player;
public: 
    explicit EnemyView(const EnemySnapshot& data);
    void update(const EnemySnapshot& data);

    std::optional<float> depth(const RenderInfo& ctx) const override;
    void draw(const RenderInfo& ctx) override;
};