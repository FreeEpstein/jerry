#pragma once

#include <string>
#include <vector>

#include "menyoo_compat/graph.hpp"

namespace menyoo {
namespace serializer {

bool save_map(const std::string& path, const std::vector<graph::EntityRecord>& entities);
bool load_map(const std::string& path, std::vector<graph::EntityRecord>& entities);

} // namespace serializer
} // namespace menyoo
