#pragma once

#include "../listeners/tool_listener.h"
#include "../../../audio/sound_emitter.h"

#include "protocol_types.h"

class SwordSwingSound : public ToolListener {
    SoundEmitter& sfx;

public:
    explicit SwordSwingSound(SoundEmitter& sfx): sfx(sfx) {}

    void on_tool_use(const ToolUse& use) override {
        if (use.tool == ItemId::Sword) sfx.cue(SoundId::SwordSwing, use.pos);
    }
};
