#pragma once

#include <string>

#include "menyoo/intent_api.hpp"

namespace menyoo {
namespace bridge_ps4 {

int spawn_vehicle(Hash model, const Transform& transform);
int spawn_ped(Hash model, const Transform& transform);
int spawn_object(Hash model, const Transform& transform);

void set_entity_transform(int handle, const Transform& transform);
void set_entity_props(int handle, const EntityProps& props);
void attach_entity(int childHandle, int parentHandle, EntityType childType, EntityType parentType, const AttachSpec& spec);
void apply_vehicle_mods(int vehicleHandle, const VehicleMods& mods);

bool save_map(const std::string& path);
bool load_map(const std::string& path);

bool capability_liveries();
bool capability_complex_paths();

} // namespace bridge_ps4
} // namespace menyoo
