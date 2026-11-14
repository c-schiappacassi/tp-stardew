#pragma once
#include <optional>
#include <vector>

#include "../../state.h"
#include "../utils.h"
#include "../render_info.h"

inline constexpr uint64_t ZONE_TIEBREAK_BASE = 1ull << 32;

class ZoneView {
public:
    virtual void draw_background(const RenderInfo& ctx) = 0;
    virtual void collect_drawables(const RenderInfo& ctx, std::vector<Drawable>& out) = 0;

    virtual ~ZoneView() = default;
};