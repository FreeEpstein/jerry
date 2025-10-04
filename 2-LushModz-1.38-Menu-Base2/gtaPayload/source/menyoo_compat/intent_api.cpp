#include "menyoo/intent_api.hpp"

#include <algorithm>
#include <unordered_map>

#include "bridge_ps4.hpp"
#include "serializer_menyoo_xml.hpp"
#include "spooner_graph.hpp"

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
        return spooner_graph::add_entity(type, model, handle, transform, props);
}

void set_entity_transform(EntityId id, const Transform& transform) {
        spooner_graph::EntityRecord record{};
        if (!spooner_graph::get_entity(id, record)) {
                return;
        }
        bridge_ps4::set_entity_transform(record.handle, transform);
        spooner_graph::update_transform(id, transform);
}

Transform get_entity_transform(EntityId id) {
        spooner_graph::EntityRecord record{};
        if (!spooner_graph::get_entity(id, record)) {
                return Transform{};
        }
        Transform transform = bridge_ps4::get_entity_transform(record.handle);
        spooner_graph::update_transform(id, transform);
        return transform;
}

void set_entity_props(EntityId id, const EntityProps& props) {
        spooner_graph::EntityRecord record{};
        if (!spooner_graph::get_entity(id, record)) {
                return;
        }
        bridge_ps4::set_entity_props(record.handle, props);
        spooner_graph::update_props(id, props);
}

EntityProps get_entity_props(EntityId id) {
        spooner_graph::EntityRecord record{};
        if (!spooner_graph::get_entity(id, record)) {
                return EntityProps{};
        }
        EntityProps props = bridge_ps4::get_entity_props(record.handle);
        props.frozen = record.props.frozen;
        spooner_graph::update_props(id, props);
        return props;
}

void attach_entity(EntityId child, EntityId parent, const AttachSpec& spec) {
        spooner_graph::EntityRecord childRecord{};
        spooner_graph::EntityRecord parentRecord{};
        if (!spooner_graph::get_entity(child, childRecord)) {
                return;
        }
        if (!spooner_graph::get_entity(parent, parentRecord)) {
                return;
        }
        bridge_ps4::attach_entity(childRecord.handle, parentRecord.handle, childRecord.type, parentRecord.type, spec);
        spooner_graph::update_attachment(child, parent, spec);
}

void detach_entity(EntityId child) {
        spooner_graph::EntityRecord childRecord{};
        if (!spooner_graph::get_entity(child, childRecord)) {
                return;
        }
        bridge_ps4::detach_entity(childRecord.handle);
        spooner_graph::clear_attachment(child);
}

bool delete_entity(EntityId id) {
        spooner_graph::EntityRecord record{};
        if (!spooner_graph::get_entity(id, record)) {
                return false;
        }
        if (!bridge_ps4::delete_entity(record.handle)) {
                return false;
        }
        spooner_graph::remove_entity(id);
        return true;
}

std::vector<EntityId> list_entities() {
        std::vector<EntityId> ids = spooner_graph::list_entities();
        ids.erase(std::remove_if(ids.begin(), ids.end(), [](EntityId id) {
                if (!bridge_ps4::entity_exists(id)) {
                        spooner_graph::remove_entity(id);
                        return true;
                }
                return false;
        }), ids.end());
        return ids;
}

bool is_valid(EntityId id) {
        if (!spooner_graph::has_entity(id)) {
                return false;
        }
        return bridge_ps4::entity_exists(id);
}

EntityType entity_type(EntityId id) {
        return spooner_graph::type(id);
}

Hash entity_model(EntityId id) {
        return spooner_graph::model(id);
}

AttachmentInfo get_attachment(EntityId id) {
        return spooner_graph::attachment(id);
}

bool save_map(const char* path) {
        return serializer::save_map(path, spooner_graph::snapshot());
}

bool load_map(const char* path) {
        std::vector<serializer::LoadedEntity> loaded;
        if (!serializer::load_map(path, loaded)) {
                return false;
        }
        std::unordered_map<EntityId, EntityId> idMap;
        for (const auto& entry : loaded) {
                EntityId newId = spawn_entity(entry.type, entry.model, entry.transform);
                if (newId == 0) {
                        continue;
                }
                set_entity_props(newId, entry.props);
                if (entry.type == EntityType::Vehicle) {
                        spooner_graph::set_vehicle_mods(newId, entry.vehicleMods);
                        bridge_ps4::apply_vehicle_mods(newId, entry.vehicleMods);
                }
                idMap[entry.legacyId] = newId;
        }
        for (const auto& entry : loaded) {
                if (!entry.attachment.attached) {
                        continue;
                }
                auto childIt = idMap.find(entry.legacyId);
                auto parentIt = idMap.find(entry.attachment.parent);
                if (childIt != idMap.end() && parentIt != idMap.end()) {
                        attach_entity(childIt->second, parentIt->second, entry.attachment.spec);
                }
        }
        return !loaded.empty();
}

bool capability_liveries() {
        return bridge_ps4::capability_liveries();
}

bool capability_complex_paths() {
        return bridge_ps4::capability_complex_paths();
}

} // namespace menyoo
