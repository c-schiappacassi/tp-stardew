#pragma once
#include <algorithm>
#include <cstddef>

#include "../../audio/audio.h"
#include "../../world/hud/volume_view.h"
#include "modal_layer.h"

class VolumeLayer : public ModalLayer {
private:
    Audio& audio;
    VolumeView view;
    std::size_t selected = 0;
    bool preview_pending = false;

    int level_of(std::size_t row) const { return row == 0 ? audio.music_level() : audio.sfx_level(); }

    void adjust(std::size_t row, int delta) {
        const int next = std::clamp(level_of(row) + delta, 0, audio_volumes::MAX_LEVEL);
        if (next == level_of(row)) return;
        if (row == 0) {
            audio.set_music_level(next);
        } else {
            audio.set_sfx_level(next);
            preview_pending = true;
        }
    }

protected:
    Focus on_key(Key key, Out&) override {
        switch (key) {
            case Key::V:
            case Key::Escape:
            case Key::Enter: return Focus::Release;
            case Key::Up: selected = 0; break;
            case Key::Down: selected = 1; break;
            case Key::Left:
            case Key::Minus: adjust(selected, -1); break;
            case Key::Right:
            case Key::Plus: adjust(selected, +1); break;
            default: break;
        }
        return Focus::Keep;
    }

    Focus on_button(std::size_t index, Out&) override {
        const std::size_t row = index / 2;
        selected = row;
        adjust(row, index % 2 == 0 ? -1 : +1);
        return Focus::Keep;
    }

public:
    explicit VolumeLayer(Audio& audio_) : audio(audio_) {}

    bool visible() const override { return focused(); }

    void on_focus() override {
        selected = 0;
        preview_pending = false;
    }

    void draw(const RenderInfo& ctx) override {
        const auto layout = VolumeView::layout(ctx);
        sync_hitboxes(ctx, layout.buttons);

        if (preview_pending) {
            preview_pending = false;
            ctx.audio.play(ctx.assets.get_sound(SoundId::SoilTilled));
        }

        view.draw(ctx, layout, {audio.music_level(), audio.sfx_level()}, audio_volumes::MAX_LEVEL, hovered_buttons());
    }
};