#pragma once
#include <optional>
#include <vector>

#include "../../state.h"
#include "../utils.h"

struct RenderInfo;

class ZoneView {
    Background back;
public:
    virtual void draw_background(const RenderInfo& ctx) = 0;
    virtual void collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) = 0;

    virtual ~ZoneView() = default;
};