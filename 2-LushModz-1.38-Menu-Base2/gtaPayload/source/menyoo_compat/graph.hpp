#pragma once

#include <vector>

#include "menyoo/intent_api.hpp"

namespace menyoo {
namespace graph {

struct EntityRecord {
        EntityId id;
        EntityType type;
        int handle;
        Transform transform;
        EntityProps props;
};

EntityId add_entity(EntityType type, int handle, const Transform& transform, const EntityProps& props);
bool get_entity(EntityId id, EntityRecord& outRecord);
void update_transform(EntityId id, const Transform& transform);
void update_props(EntityId id, const EntityProps& props);
std::vector<EntityRecord> snapshot();

} // namespace graph
} // namespace menyoo
