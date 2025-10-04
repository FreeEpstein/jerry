#include "bridge_ps4.hpp"

#include <algorithm>

#include "natives.h"

namespace menyoo {
namespace {

constexpr int kMaxModelTries = 100;

bool ensure_model_loaded(Hash model) {
        if (model == 0u) {
                return false;
        }
        if (HAS_MODEL_LOADED(model)) {
                return true;
        }
        for (int attempt = 0; attempt < kMaxModelTries; ++attempt) {
                REQUEST_MODEL(model);
                if (HAS_MODEL_LOADED(model)) {
                        return true;
                }
                WAIT(0);
        }
        return HAS_MODEL_LOADED(model);
}

float heading_from_transform(const Transform& transform) {
        return transform.rot.z;
}

void apply_proofs(int handle, bool enabled) {
        SET_ENTITY_PROOFS(handle, enabled, enabled, enabled, enabled, enabled, enabled, enabled, enabled);
        SET_ENTITY_CAN_BE_DAMAGED(handle, !enabled);
        SET_ENTITY_INVINCIBLE(handle, enabled);
}

} // namespace

namespace bridge_ps4 {

int spawn_vehicle(Hash model, const Transform& transform) {
        if (!ensure_model_loaded(model)) {
                return 0;
        }
        const auto& pos = transform.pos;
        int handle = CREATE_VEHICLE(model, pos.x, pos.y, pos.z, heading_from_transform(transform), true, true, 1);
        SET_MODEL_AS_NO_LONGER_NEEDED(model);
        return handle;
}

int spawn_ped(Hash model, const Transform& transform) {
        if (!ensure_model_loaded(model)) {
                return 0;
        }
        const auto& pos = transform.pos;
        int handle = CREATE_PED(26, model, pos.x, pos.y, pos.z, heading_from_transform(transform), true, true);
        SET_MODEL_AS_NO_LONGER_NEEDED(model);
        return handle;
}

int spawn_object(Hash model, const Transform& transform) {
        if (!ensure_model_loaded(model)) {
                return 0;
        }
        const auto& pos = transform.pos;
        int handle = CREATE_OBJECT(model, pos.x, pos.y, pos.z, true, true, true);
        SET_MODEL_AS_NO_LONGER_NEEDED(model);
        return handle;
}

void set_entity_transform(int handle, const Transform& transform) {
        const auto& pos = transform.pos;
        const auto& rot = transform.rot;
        SET_ENTITY_COORDS(handle, pos.x, pos.y, pos.z, false, false, false, true);
        SET_ENTITY_ROTATION(handle, rot.x, rot.y, rot.z, 2, true);
}

void set_entity_props(int handle, const EntityProps& props) {
        SET_ENTITY_VISIBLE(handle, props.visible, false);
        SET_ENTITY_COLLISION(handle, props.collision, true);
        apply_proofs(handle, props.invincible);
        FREEZE_ENTITY_POSITION(handle, props.frozen);
        const int alpha = std::max(0, std::min(255, props.alpha));
        SET_ENTITY_ALPHA(handle, alpha, props.visible);
}

EntityProps get_entity_props(int handle) {
        EntityProps props{};
        props.visible = IS_ENTITY_VISIBLE(handle);
        props.collision = !GET_ENTITY_COLLISION_DISABLED(handle);
        props.invincible = !_GET_ENTITY_CAN_BE_DAMAGED(handle);
        props.alpha = GET_ENTITY_ALPHA(handle);
        props.frozen = false;
        return props;
}

Transform get_entity_transform(int handle) {
        Transform transform{};
        vector3 pos = GET_ENTITY_COORDS(handle, true);
        vector3 rot = GET_ENTITY_ROTATION(handle, 2);
        transform.pos = {pos.x, pos.y, pos.z};
        transform.rot = {rot.x, rot.y, rot.z};
        return transform;
}

void attach_entity(int childHandle, int parentHandle, EntityType childType, EntityType parentType, const AttachSpec& spec) {
        int boneIndex = 0;
        if (!spec.bone.empty()) {
                boneIndex = GET_ENTITY_BONE_INDEX_BY_NAME(parentHandle, spec.bone.c_str());
        }
        ATTACH_ENTITY_TO_ENTITY(childHandle,
                                parentHandle,
                                boneIndex,
                                spec.offset.x,
                                spec.offset.y,
                                spec.offset.z,
                                spec.rot.x,
                                spec.rot.y,
                                spec.rot.z,
                                false,
                                spec.softPin,
                                spec.collisions,
                                childType == EntityType::Ped,
                                0,
                                true);
}

void detach_entity(int childHandle) {
        DETACH_ENTITY(childHandle, true, true);
}

bool delete_entity(int handle) {
        if (!DOES_ENTITY_EXIST(handle)) {
                return false;
        }
        SET_ENTITY_AS_MISSION_ENTITY(handle, true, true);
        int entity = handle;
        DELETE_ENTITY(&entity);
        return true;
}

bool entity_exists(int handle) {
        return DOES_ENTITY_EXIST(handle);
}

void apply_vehicle_mods(int vehicleHandle, const VehicleMods& mods) {
        SET_VEHICLE_MOD_KIT(vehicleHandle, 0);
        if (mods.primaryColor >= 0 && mods.secondaryColor >= 0) {
                SET_VEHICLE_COLOURS(vehicleHandle, mods.primaryColor, mods.secondaryColor);
        }
        if (mods.pearlescent >= 0 || mods.wheelColor >= 0) {
                int pearl = mods.pearlescent >= 0 ? mods.pearlescent : 0;
                int wheel = mods.wheelColor >= 0 ? mods.wheelColor : 0;
                SET_VEHICLE_EXTRA_COLOURS(vehicleHandle, pearl, wheel);
        }
        for (int i = 0; i < 4; ++i) {
                _SET_VEHICLE_NEON_LIGHT_ENABLED(vehicleHandle, i, mods.neon[i]);
        }
        for (int extra : mods.extrasOn) {
                SET_VEHICLE_EXTRA(vehicleHandle, extra, false);
        }
}

bool save_map(const char*) {
        return false;
}

bool load_map(const char*) {
        return false;
}

bool capability_liveries() {
        return false;
}

bool capability_complex_paths() {
        return false;
}

} // namespace bridge_ps4
} // namespace menyoo
