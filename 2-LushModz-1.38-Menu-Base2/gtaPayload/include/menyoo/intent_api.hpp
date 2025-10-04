#pragma once

#include <string>
#include <vector>

namespace menyoo {

struct Vec3 {
        float x;
        float y;
        float z;
};

struct Transform {
        Vec3 pos;
        Vec3 rot;
};

enum class EntityType {
        Ped,
        Vehicle,
        Object
};

using Hash = unsigned int;
using EntityId = int;
using VehicleId = int;
using PedId = int;

struct EntityProps {
        bool visible = true;
        bool collision = true;
        bool invincible = false;
        bool frozen = false;
        int alpha = 255;
};

struct VehicleMods {
        int engine = 0;
        int brakes = 0;
        int transmission = 0;
        int suspension = 0;
        int armor = 0;
        int primaryColor = -1;
        int secondaryColor = -1;
        int pearlescent = -1;
        int wheelColor = -1;
        bool neon[4] = {false, false, false, false};
        std::vector<int> extrasOn;
};

struct AttachSpec {
        std::string bone;
        Vec3 offset{};
        Vec3 rot{};
        bool softPin = false;
        bool collisions = false;
};

EntityId spawn_entity(EntityType type, Hash model, const Transform& transform);
void set_entity_transform(EntityId id, const Transform& transform);
void set_entity_props(EntityId id, const EntityProps& props);
void attach_entity(EntityId child, EntityId parent, const AttachSpec& spec);
void apply_vehicle_mods(VehicleId vehicle, const VehicleMods& mods);

bool save_map(const std::string& path);
bool load_map(const std::string& path);

bool capability_liveries();
bool capability_complex_paths();

} // namespace menyoo
