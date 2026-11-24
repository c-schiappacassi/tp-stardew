#pragma once

#include <filesystem>
#include <ostream>

#include "common/map/map_data.h"

class MapWriter {
public:
    void write(std::ostream& out, const MapData& map) const;
    bool save(const MapData& map, const std::filesystem::path& path) const;

private:
    static void writeZone(std::ostream& out, const EditorZoneData& zone, bool withBuildings);
};