#pragma once

#include <filesystem>

#include <SDL2pp/SDL2pp.hh>

#include "asset_tables.h"

AssetTables load_assets(SDL2pp::Renderer& renderer, const std::filesystem::path& assets_root);
