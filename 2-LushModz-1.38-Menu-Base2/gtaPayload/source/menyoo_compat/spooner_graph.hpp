#pragma once

#include <vector>

#include "menyoo/spooner.hpp"

namespace menyoo {
namespace spooner_graph {

struct EntityRecord {
        EntityId id;
        EntityType type;
        Hash model;
        int handle;
        Transform transform;
        EntityProps props;
        AttachmentInfo attachment;
        VehicleMods vehicleMods;
};

EntityId add_entity(EntityType type, Hash model, int handle, const Transform& transform, const EntityProps& props);
bool has_entity(EntityId id);
bool get_entity(EntityId id, EntityRecord& outRecord);
void update_transform(EntityId id, const Transform& transform);
void update_props(EntityId id, const EntityProps& props);
void update_attachment(EntityId child, EntityId parent, const AttachSpec& spec);
void clear_attachment(EntityId child);
void set_vehicle_mods(EntityId id, const VehicleMods& mods);
VehicleMods get_vehicle_mods(EntityId id);
Hash model(EntityId id);
EntityType type(EntityId id);
bool remove_entity(EntityId id);
std::vector<EntityId> list_entities();
std::vector<EntityRecord> snapshot();
AttachmentInfo attachment(EntityId id);

} // namespace spooner_graph
} // namespace menyoo
