#pragma once
#include <optional>
#include <vector>

#include "../../state.h"
#include "../click_target.h"
#include "../utils.h"

struct RenderInfo;

class ZoneView {
public:
    virtual void draw_background(const RenderInfo& ctx) = 0;
    virtual void collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) = 0;

    virtual std::optional<ClickTarget> pick_target(int world_x, int world_y) const;
    virtual ~ZoneView() = default;
};