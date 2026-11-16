#pragma once

#include <memory>
#include <vector>

#include "dtos.h"
#include "input_event.h"

class State;
struct RenderInfo;

enum class Focus { Keep, Release };

class UiLayer {
    friend class UiManager;
    bool has_focus = false;

public:
    using Out = std::vector<std::shared_ptr<CommandDTO>>;

    virtual ~UiLayer() = default;

    virtual bool visible() const { return true; }

    virtual void update(const State&) {}
    virtual void draw(const RenderInfo&) {}

    virtual Focus handle(const InputEvent&, Out&) { return Focus::Keep; }

    virtual void on_focus() {}
    virtual void on_blur() {}

    bool focused() const { return has_focus; }
};
