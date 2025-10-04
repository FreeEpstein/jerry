#include "spooner_graph.hpp"

#include <unordered_map>

namespace menyoo {
namespace {

struct InternalRecord {
        EntityType type;
        Hash model;
        int handle;
        Transform transform;
        EntityProps props;
        AttachmentInfo attachment;
        VehicleMods vehicleMods;
};

std::unordered_map<EntityId, InternalRecord> g_records;

} // namespace

namespace spooner_graph {

EntityId add_entity(EntityType type, Hash model, int handle, const Transform& transform, const EntityProps& props) {
        if (handle == 0) {
                return 0;
        }
        InternalRecord record{};
        record.type = type;
        record.model = model;
        record.handle = handle;
        record.transform = transform;
        record.props = props;
        g_records[handle] = record;
        return handle;
}

bool has_entity(EntityId id) {
        return g_records.find(id) != g_records.end();
}

bool get_entity(EntityId id, EntityRecord& outRecord) {
        auto it = g_records.find(id);
        if (it == g_records.end()) {
                return false;
        }
        const InternalRecord& record = it->second;
        outRecord = {id, record.type, record.model, record.handle, record.transform, record.props, record.attachment, record.vehicleMods};
        return true;
}

void update_transform(EntityId id, const Transform& transform) {
        auto it = g_records.find(id);
        if (it != g_records.end()) {
                it->second.transform = transform;
        }
}

void update_props(EntityId id, const EntityProps& props) {
        auto it = g_records.find(id);
        if (it != g_records.end()) {
                it->second.props = props;
        }
}

void update_attachment(EntityId child, EntityId parent, const AttachSpec& spec) {
        auto it = g_records.find(child);
        if (it != g_records.end()) {
                it->second.attachment.attached = true;
                it->second.attachment.parent = parent;
                it->second.attachment.spec = spec;
        }
}

void clear_attachment(EntityId child) {
        auto it = g_records.find(child);
        if (it != g_records.end()) {
                it->second.attachment = AttachmentInfo{};
        }
}

void set_vehicle_mods(EntityId id, const VehicleMods& mods) {
        auto it = g_records.find(id);
        if (it != g_records.end()) {
                it->second.vehicleMods = mods;
        }
}

VehicleMods get_vehicle_mods(EntityId id) {
        auto it = g_records.find(id);
        if (it != g_records.end()) {
                return it->second.vehicleMods;
        }
        return VehicleMods{};
}

Hash model(EntityId id) {
        auto it = g_records.find(id);
        if (it != g_records.end()) {
                return it->second.model;
        }
        return 0;
}

EntityType type(EntityId id) {
        auto it = g_records.find(id);
        if (it != g_records.end()) {
                return it->second.type;
        }
        return EntityType::Object;
}

bool remove_entity(EntityId id) {
        auto it = g_records.find(id);
        if (it == g_records.end()) {
                return false;
        }
        g_records.erase(it);
        for (auto& pair : g_records) {
                auto& attachment = pair.second.attachment;
                if (attachment.attached && attachment.parent == id) {
                        attachment = AttachmentInfo{};
                }
        }
        return true;
}

std::vector<EntityId> list_entities() {
        std::vector<EntityId> ids;
        ids.reserve(g_records.size());
        for (const auto& pair : g_records) {
                ids.push_back(pair.first);
        }
        return ids;
}

std::vector<EntityRecord> snapshot() {
        std::vector<EntityRecord> result;
        result.reserve(g_records.size());
        for (const auto& pair : g_records) {
                const InternalRecord& record = pair.second;
                result.push_back({pair.first, record.type, record.model, record.handle, record.transform, record.props, record.attachment, record.vehicleMods});
        }
        return result;
}

AttachmentInfo attachment(EntityId id) {
        auto it = g_records.find(id);
        if (it != g_records.end()) {
                return it->second.attachment;
        }
        return AttachmentInfo{};
}

} // namespace spooner_graph
} // namespace menyoo
