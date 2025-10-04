#include "menyoo_compat/bridge_ps4.hpp"

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
        SET_ENTITY_INVINCIBLE(handle, props.invincible);
        FREEZE_ENTITY_POSITION(handle, props.frozen);
        const int alpha = std::max(0, std::min(255, props.alpha));
        SET_ENTITY_ALPHA(handle, alpha, props.visible);
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

void apply_vehicle_mods(int vehicleHandle, const VehicleMods& mods) {
        SET_VEHICLE_MOD_KIT(vehicleHandle, 0);
        if (mods.engine >= 0) {
                SET_VEHICLE_MOD(vehicleHandle, 11, mods.engine, false);
        }
        if (mods.brakes >= 0) {
                SET_VEHICLE_MOD(vehicleHandle, 12, mods.brakes, false);
        }
        if (mods.transmission >= 0) {
                SET_VEHICLE_MOD(vehicleHandle, 13, mods.transmission, false);
        }
        if (mods.suspension >= 0) {
                SET_VEHICLE_MOD(vehicleHandle, 15, mods.suspension, false);
        }
        if (mods.armor >= 0) {
                SET_VEHICLE_MOD(vehicleHandle, 16, mods.armor, false);
        }
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

bool save_map(const std::string&) {
        return false;
}

bool load_map(const std::string&) {
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
