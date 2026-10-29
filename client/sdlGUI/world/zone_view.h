#pragma once
#include <vector>
#include "../utils.h"
#include "../../state.h"

struct RenderInfo;

class ZoneView {
public:
    virtual void draw_background(const RenderInfo& ctx) = 0;
    virtual void collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) = 0;
};