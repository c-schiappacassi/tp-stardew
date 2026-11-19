#include "minimap_view.h"

std::optional<Area> MinimapView::area_of(const PlayerData& p) {
    switch (p.currentZone) {
        case Zone::Farm: return Area::Farm;
        case Zone::Town: return Area::Town;
        case Zone::Dungeon: return Area::Mines;
        case Zone::InsideHouse:
            return p.interior && p.interior->interior_id != InteriorId::PlayerHouse ? Area::Town : Area::Farm;
    }
    return std::nullopt;
}

SDL2pp::Rect MinimapView::to_screen(const SDL2pp::Rect& r, const SDL2pp::Rect& map, const SDL2pp::Point& tex) {
    const int x0 = map.x + r.x * map.w / tex.x;
    const int y0 = map.y + r.y * map.h / tex.y;
    const int x1 = map.x + (r.x + r.w) * map.w / tex.x;
    const int y1 = map.y + (r.y + r.h) * map.h / tex.y;
    return SDL2pp::Rect(x0, y0, x1 - x0, y1 - y0);
}

SDL2pp::Rect MinimapView::fit(const SDL2pp::Point& screen, const SDL2pp::Point& tex) {
    const SDL2pp::Point size = fit_size(tex.x, tex.y, static_cast<int>(screen.x * SCREEN_FILL), static_cast<int>(screen.y * SCREEN_FILL));
    return centered_rect(screen, size.x, size.y);
}

void MinimapView::draw_head(const RenderInfo& ctx, const SDL2pp::Point& center, int size, const Marker& m, const Sprite& head) {
    const int w = size;
    const int h = std::max(1, w * head.height / head.width);
    const SDL2pp::Rect head_rect = rect_centered_on(center, w, h);
    draw_sprite_in(ctx, head, head_rect);

    const int hair_h = head.height - farmer::HAIR_BASE.y;
    const SDL2pp::Rect src = farmer::hair_src(m.hair_model, 0, hair_h);
    const SDL2pp::Rect dst(head_rect.x + farmer::HAIR_BASE.x * w / head.width, head_rect.y + farmer::HAIR_BASE.y * h / head.height, w, hair_h * h / head.height);
    draw_scaled(ctx, SpriteId::Hairstyles, src, dst, false, m.hair_color);
}

void MinimapView::update(const std::map<std::string, PlayerData>& players) {
    for (auto& area_markers: markers) {
        area_markers.clear();
    }
    for (const auto& [name, p]: players) {
        if (const auto area = area_of(p)) {
            auto& area_markers = markers[static_cast<std::size_t>(*area)];
            if (area_markers.size() < MAX_PLAYERS) {
                area_markers.push_back({p.hairModel, to_sdl(p.hairColor)});
            }
        }
    }
}

void MinimapView::draw(const RenderInfo& ctx) const {
    const SDL2pp::Point screen = ctx.r.GetOutputSize();

    dim_screen(ctx, DIM_ALPHA);

    const SDL2pp::Point tex = ctx.assets.get_texture(SpriteId::Minimap).GetSize();
    if (tex.x <= 0 || tex.y <= 0)  return;
    const SDL2pp::Rect map = fit(screen, tex);
    draw_scaled(ctx, SpriteId::Minimap, SDL2pp::Rect(0, 0, tex.x, tex.y), map);

    const Sprite& head = ctx.assets.get_sprite(UiSpriteId::PlayerHead);
    for (std::size_t a = 0; a < markers.size(); ++a) {
        const auto& area_markers = markers[a];
        const SDL2pp::Rect zone = to_screen(REGIONS[a], map, tex);
        const int size = std::max(1, static_cast<int>(HEAD_SCALE * std::min(zone.w, zone.h)));

        for (std::size_t i = 0; i < area_markers.size(); ++i) {
            const Slot& s = LAYOUTS[area_markers.size() - 1][i];
            const SDL2pp::Point center(zone.x + static_cast<int>(s.x * zone.w), zone.y + static_cast<int>(s.y * zone.h));
            draw_head(ctx, center, size, area_markers[i], head);
        }
    }
}
