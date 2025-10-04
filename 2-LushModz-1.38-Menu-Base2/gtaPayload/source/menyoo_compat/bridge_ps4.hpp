#pragma once

#include <string>

#include "menyoo/spooner.hpp"

namespace menyoo {
namespace bridge_ps4 {

int spawn_vehicle(Hash model, const Transform& transform);
int spawn_ped(Hash model, const Transform& transform);
int spawn_object(Hash model, const Transform& transform);

void set_entity_transform(int handle, const Transform& transform);
void set_entity_props(int handle, const EntityProps& props);
EntityProps get_entity_props(int handle);
Transform get_entity_transform(int handle);
void attach_entity(int childHandle, int parentHandle, EntityType childType, EntityType parentType, const AttachSpec& spec);
void detach_entity(int childHandle);
bool delete_entity(int handle);
bool entity_exists(int handle);
void apply_vehicle_mods(int vehicleHandle, const VehicleMods& mods);

bool save_map(const char* path);
bool load_map(const char* path);

bool capability_liveries();
bool capability_complex_paths();

} // namespace bridge_ps4
} // namespace menyoo
