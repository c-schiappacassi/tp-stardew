#pragma once
#include <cstdint>

#include "../../render_info.h"

class BarView {
private:
    char label;
    int order;
    bool hide_when_full;

    uint16_t value = 0;
    uint16_t max_value = 0;

public:
    explicit BarView(char label, int order = 0, bool hide_when_full = false)
        : label(label), order(order), hide_when_full(hide_when_full) {}

    void update(uint16_t current, uint16_t maximum) {
        value = current;
        max_value = maximum;
    }

    void draw(const RenderInfo& ctx) const;
};