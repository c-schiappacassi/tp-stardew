#pragma once
#include <optional>

struct RenderInfo;

class EntityView {
public:
    virtual std::optional<float> depth(const RenderInfo& ctx) const = 0;
    virtual void draw(const RenderInfo& ctx) = 0;
    virtual ~EntityView() = default;
};