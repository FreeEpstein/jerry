#pragma once

#include <string>
#include <vector>

namespace menyoo {

struct Vec3 {
        float x;
        float y;
        float z;

        Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
        Vec3(float ix, float iy, float iz) : x(ix), y(iy), z(iz) {}
};

struct Transform {
        Vec3 pos;
        Vec3 rot;

        Transform() = default;
        Transform(const Vec3& position, const Vec3& rotation) : pos(position), rot(rotation) {}
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
        bool collisions = false;
        bool softPin = false;
};

struct AttachmentInfo {
        bool attached = false;
        EntityId parent = 0;
        AttachSpec spec{};
};

EntityId spawn_entity(EntityType type, Hash model, const Transform& transform);
void set_entity_transform(EntityId id, const Transform& transform);
Transform get_entity_transform(EntityId id);
void set_entity_props(EntityId id, const EntityProps& props);
EntityProps get_entity_props(EntityId id);
void attach_entity(EntityId child, EntityId parent, const AttachSpec& spec);
void detach_entity(EntityId child);
bool delete_entity(EntityId id);

std::vector<EntityId> list_entities();
bool is_valid(EntityId id);
EntityType entity_type(EntityId id);
Hash entity_model(EntityId id);
AttachmentInfo get_attachment(EntityId id);

bool save_map(const char* path);
bool load_map(const char* path);

bool capability_liveries();
bool capability_complex_paths();

} // namespace menyoo
