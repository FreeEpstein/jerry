#if MENUYO_TESTS

#include "menyoo/intent_api.hpp"

namespace {

constexpr menyoo::Hash kAdderHash = 0xB779A091;
constexpr menyoo::Hash kPropHash = 0x5D6B5F0B;

} // namespace

extern "C" void run_spooner_min_test() {
        using namespace menyoo;

        Transform vehicleTransform{};
        vehicleTransform.pos = {0.0f, 0.0f, 75.0f};
        vehicleTransform.rot = {0.0f, 0.0f, 0.0f};
        EntityId vehicleId = spawn_entity(EntityType::Vehicle, kAdderHash, vehicleTransform);
        if (vehicleId == 0) {
                return;
        }
        set_entity_transform(vehicleId, vehicleTransform);

        Transform propTransform{};
        propTransform.pos = {0.0f, 0.0f, 75.0f};
        propTransform.rot = {0.0f, 0.0f, 0.0f};
        EntityId propId = spawn_entity(EntityType::Object, kPropHash, propTransform);
        if (propId != 0) {
                AttachSpec spec;
                spec.bone = "chassis";
                spec.offset = {0.0f, 1.0f, 0.4f};
                attach_entity(propId, vehicleId, spec);
        }

        EntityProps props;
        props.invincible = true;
        props.frozen = true;
        props.alpha = 200;
        set_entity_props(vehicleId, props);

        save_map("tmp_map.xml");
        load_map("tmp_map.xml");
}

#endif // MENUYO_TESTS
