#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "farm/animals/animal_species.h"
#include "protocol_types.h"

namespace shop_products {
std::string_view name(ItemId item);
std::optional<ItemId> item(const std::string& name);
std::vector<std::string_view> itemNames();
std::optional<ItemId> cropItem(CropType crop);
std::optional<AnimalSpecies> animal(const std::string& name);
std::optional<AnimalType> animalVariant(const std::string& name);
std::string_view merchantName(NpcId merchant);
std::optional<NpcId> merchantForInterior(InteriorId interior);
std::optional<uint64_t> salePrice(uint32_t basePrice, Quality quality);
std::optional<uint64_t> salePriceQuarters(uint32_t basePrice, Quality quality);
}
