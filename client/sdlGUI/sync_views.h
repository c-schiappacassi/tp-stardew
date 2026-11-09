#pragma once
#include <unordered_set>
#include "utils.h"
#include "render_info.h"

template <typename Views, typename Datas, typename KeyOf>
void sync_views(Views& views, const Datas& datas, KeyOf key_of) {
    using Key = typename Views::key_type;
    std::unordered_set<Key> alive;

    for (const auto& data : datas) {
        const Key key = key_of(data);
        alive.insert(key);

        auto [it, inserted] = views.try_emplace(key, data);
        if (!inserted) {
            it->second.update(data);
        }
    }

    std::erase_if(views, [&](const auto& entry) {
        return alive.find(entry.first) == alive.end();
    });
}

template <typename Views, typename DataMap>
void sync_views_map(Views& views, const DataMap& datas) {
    for (const auto& [key, data] : datas) {
        auto [it, inserted] = views.try_emplace(key, data);
        if (!inserted) it->second.update(data);
    }
    std::erase_if(views, [&](const auto& entry) {
        return datas.find(entry.first) == datas.end();
    });
}

template <typename Views>
void collect_from(const RenderInfo& ctx, Views& views, std::vector<Drawable>& out, uint64_t& tie) {
    for (auto& [key, view] : views) {
        if (const auto depth = view.depth(ctx)) {
            out.push_back(Drawable{*depth, tie++, &view});
        }
    }
}