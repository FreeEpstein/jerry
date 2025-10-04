#include "menyoo_compat/serializer_menyoo_xml.hpp"

namespace menyoo {
namespace serializer {

bool save_map(const std::string&, const std::vector<graph::EntityRecord>&) {
        return false; // TODO: Implement Menyoo XML export.
}

bool load_map(const std::string&, std::vector<graph::EntityRecord>&) {
        return false; // TODO: Implement Menyoo XML import.
}

} // namespace serializer
} // namespace menyoo
