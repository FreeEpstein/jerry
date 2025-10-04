#pragma once

#include <string>
#include <vector>

#include "menyoo/spooner.hpp"
#include "spooner_graph.hpp"

namespace menyoo {
namespace serializer {

struct LoadedEntity {
        EntityId legacyId = 0;
        EntityType type = EntityType::Object;
        Hash model = 0;
        Transform transform{};
        EntityProps props{};
        AttachmentInfo attachment{};
        VehicleMods vehicleMods{};
};

bool save_map(const char* path, const std::vector<spooner_graph::EntityRecord>& entities);
bool load_map(const char* path, std::vector<LoadedEntity>& entities);

} // namespace serializer
} // namespace menyoo
