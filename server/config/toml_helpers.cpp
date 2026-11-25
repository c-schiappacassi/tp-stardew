#include "config/toml_helpers.h"

#include "server_constants.h"

#include <stdexcept>

namespace toml_helpers {

[[noreturn]] void invalid(const std::string& field,
                          const std::string& reason) {
    throw std::invalid_argument(field + server_constants::COLON_SPACE + reason);
}

const toml::table& tableAt(const toml::node* node,
                           std::string_view field) {
    if (!node || !node->is_table()) {
        invalid(std::string(field), server_constants::EXPECTED_A_TABLE);
    }

    return *node->as_table();
}

int64_t integer(const toml::table& table,
                std::string_view key,
                int64_t maximum,
                const std::string& field) {
    const auto* node = table.get(key);

    if (!node || !node->is_integer()) {
        invalid(field, server_constants::EXPECTED_AN_INTEGER);
    }

    const auto value = *node->value<int64_t>();

    if (value < 0 || value > maximum) {
        invalid(field, server_constants::INTEGER_OUT_OF_RANGE);
    }

    return value;
}

bool boolean(const toml::table& table,
             std::string_view key,
             const std::string& field) {
    const auto* node = table.get(key);

    if (!node || !node->is_boolean()) {
        invalid(field, server_constants::EXPECTED_A_BOOLEAN);
    }

    return *node->value<bool>();
}

std::string string(const toml::table& table,
                   std::string_view key,
                   const std::string& field) {
    const auto* node = table.get(key);

    if (!node || !node->is_string()) {
        invalid(field, server_constants::EXPECTED_A_STRING);
    }

    return *node->value<std::string>();
}

bool optionalBool(const toml::table& table,
                  std::string_view key,
                  const std::string& field) {
    const auto* node = table.get(key);

    if (!node) {
        return false;
    }

    if (!node->is_boolean()) {
        invalid(field, server_constants::EXPECTED_A_BOOLEAN);
    }

    return *node->value<bool>();
}

}  // namespace toml_helpers
