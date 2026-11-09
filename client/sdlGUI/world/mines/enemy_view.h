#pragma once
#include <optional>

#include "../../utils.h"
#include "../entity_view.h"
#include "../../animator.h"

#include "position.h"
#include "protocol_types.h"
#include "dtos.h"

class EnemyView: public EntityView {
private:
    Position pos{0, 0};
    EnemyType type;
       
    Direction facing = Direction::Down;
    EnemyState state;
    
    Animator anim;
    std::optional<ClipId> current_clip;
    long long last_it = -1;

public:
    explicit EnemyView(const EnemyData& data);

    void update(const EnemyData& data);

    std::optional<float> depth(const RenderInfo& ctx) const override;
    void draw(const RenderInfo& ctx) override;
};