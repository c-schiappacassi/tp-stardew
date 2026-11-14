#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include <toml++/toml.hpp>

namespace toml_helpers {

[[noreturn]] void invalid(const std::string& field,
                          const std::string& reason);

const toml::table& tableAt(const toml::node* node,
                           std::string_view field);

int64_t integer(const toml::table& table,
                std::string_view key,
                int64_t maximum,
                const std::string& field);

bool boolean(const toml::table& table,
             std::string_view key,
             const std::string& field);

std::string string(const toml::table& table,
                   std::string_view key,
                   const std::string& field);

bool optionalBool(const toml::table& table,
                  std::string_view key,
                  const std::string& field);

}  // namespace toml_helpers
