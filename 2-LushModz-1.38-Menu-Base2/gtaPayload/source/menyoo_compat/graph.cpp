#include "menyoo_compat/graph.hpp"

#include <unordered_map>

namespace menyoo {
namespace {

struct InternalRecord {
        EntityType type;
        int handle;
        Transform transform;
        EntityProps props;
};

std::unordered_map<EntityId, InternalRecord> g_records;

} // namespace

namespace graph {

EntityId add_entity(EntityType type, int handle, const Transform& transform, const EntityProps& props) {
        if (handle == 0) {
                return 0;
        }
        InternalRecord record{type, handle, transform, props};
        g_records[handle] = record;
        return handle;
}

bool get_entity(EntityId id, EntityRecord& outRecord) {
        auto it = g_records.find(id);
        if (it == g_records.end()) {
                return false;
        }
        outRecord = {id, it->second.type, it->second.handle, it->second.transform, it->second.props};
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

std::vector<EntityRecord> snapshot() {
        std::vector<EntityRecord> result;
        result.reserve(g_records.size());
        for (const auto& pair : g_records) {
                const auto& value = pair.second;
                result.push_back({pair.first, value.type, value.handle, value.transform, value.props});
        }
        return result;
}

} // namespace graph
} // namespace menyoo
