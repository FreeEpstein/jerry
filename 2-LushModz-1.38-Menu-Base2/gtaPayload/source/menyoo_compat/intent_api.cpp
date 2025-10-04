#include "menyoo/intent_api.hpp"

#include "menyoo_compat/bridge_ps4.hpp"
#include "menyoo_compat/graph.hpp"
#include "menyoo_compat/serializer_menyoo_xml.hpp"

namespace menyoo {

namespace {

EntityProps default_props_for_type(EntityType) {
        return EntityProps{};
}

} // namespace

EntityId spawn_entity(EntityType type, Hash model, const Transform& transform) {
        int handle = 0;
        switch (type) {
        case EntityType::Vehicle:
                handle = bridge_ps4::spawn_vehicle(model, transform);
                break;
        case EntityType::Ped:
                handle = bridge_ps4::spawn_ped(model, transform);
                break;
        case EntityType::Object:
                handle = bridge_ps4::spawn_object(model, transform);
                break;
        }

        if (handle == 0) {
                return 0;
        }

        EntityProps props = default_props_for_type(type);
        bridge_ps4::set_entity_transform(handle, transform);
        bridge_ps4::set_entity_props(handle, props);
        return graph::add_entity(type, handle, transform, props);
}

void set_entity_transform(EntityId id, const Transform& transform) {
        graph::EntityRecord record{};
        if (!graph::get_entity(id, record)) {
                return;
        }
        bridge_ps4::set_entity_transform(record.handle, transform);
        graph::update_transform(id, transform);
}

void set_entity_props(EntityId id, const EntityProps& props) {
        graph::EntityRecord record{};
        if (!graph::get_entity(id, record)) {
                return;
        }
        bridge_ps4::set_entity_props(record.handle, props);
        graph::update_props(id, props);
}

void attach_entity(EntityId child, EntityId parent, const AttachSpec& spec) {
        graph::EntityRecord childRecord{};
        graph::EntityRecord parentRecord{};
        if (!graph::get_entity(child, childRecord)) {
                return;
        }
        if (!graph::get_entity(parent, parentRecord)) {
                return;
        }
        bridge_ps4::attach_entity(childRecord.handle, parentRecord.handle, childRecord.type, parentRecord.type, spec);
}

void apply_vehicle_mods(VehicleId vehicle, const VehicleMods& mods) {
        graph::EntityRecord record{};
        if (!graph::get_entity(vehicle, record)) {
                return;
        }
        if (record.type != EntityType::Vehicle) {
                return;
        }
        bridge_ps4::apply_vehicle_mods(record.handle, mods);
}

bool save_map(const std::string& path) {
        return serializer::save_map(path, graph::snapshot());
}

bool load_map(const std::string& path) {
        auto entities = graph::snapshot();
        return serializer::load_map(path, entities);
}

bool capability_liveries() {
        return bridge_ps4::capability_liveries();
}

bool capability_complex_paths() {
        return bridge_ps4::capability_complex_paths();
}

} // namespace menyoo
