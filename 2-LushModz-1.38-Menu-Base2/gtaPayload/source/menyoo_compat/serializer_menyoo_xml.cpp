#include "serializer_menyoo_xml.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <sstream>

namespace menyoo {
namespace {

std::string type_to_string(EntityType type) {
        switch (type) {
        case EntityType::Vehicle:
                return "vehicle";
        case EntityType::Ped:
                return "ped";
        case EntityType::Object:
        default:
                return "object";
        }
}

EntityType type_from_string(const std::string& value) {
        if (value == "vehicle") {
                return EntityType::Vehicle;
        }
        if (value == "ped") {
                return EntityType::Ped;
        }
        return EntityType::Object;
}

bool parse_bool(const std::string& value) {
        if (value.empty()) {
                return false;
        }
        if (value == "1") {
                return true;
        }
        if (value == "0") {
                return false;
        }
        std::string lowered = value;
        for (char& ch : lowered) {
                if (ch >= 'A' && ch <= 'Z') {
                        ch = static_cast<char>(ch - 'A' + 'a');
                }
        }
        return lowered == "true";
}

float parse_float(const std::string& value) {
        return value.empty() ? 0.0f : static_cast<float>(std::atof(value.c_str()));
}

int parse_int(const std::string& value) {
        if (value.empty()) {
                return 0;
        }
        return std::atoi(value.c_str());
}

Hash parse_hash(const std::string& value) {
        if (value.empty()) {
                return 0;
        }
        if (value.size() > 2 && value[0] == '0' && (value[1] == 'x' || value[1] == 'X')) {
                return static_cast<Hash>(std::strtoul(value.c_str(), nullptr, 16));
        }
        return static_cast<Hash>(std::strtoul(value.c_str(), nullptr, 10));
}

std::string get_attribute(const std::string& tag, const char* attribute) {
        std::string pattern = std::string(attribute) + "=\"";
        auto start = tag.find(pattern);
        if (start == std::string::npos) {
                return std::string();
        }
        start += pattern.size();
        auto end = tag.find('"', start);
        if (end == std::string::npos) {
                return std::string();
        }
        return tag.substr(start, end - start);
}

std::string get_tag_content(const std::string& block, const char* tag) {
        std::string open = std::string("<") + tag + ">";
        std::string close = std::string("</") + tag + ">";
        auto start = block.find(open);
        if (start == std::string::npos) {
                return std::string();
        }
        start += open.size();
        auto end = block.find(close, start);
        if (end == std::string::npos) {
                return std::string();
        }
        return block.substr(start, end - start);
}

std::string get_tag_segment(const std::string& block, const char* tag) {
        std::string open = std::string("<") + tag;
        auto start = block.find(open);
        if (start == std::string::npos) {
                return std::string();
        }
        auto end = block.find('>', start);
        if (end == std::string::npos) {
                return std::string();
        }
        return block.substr(start, end - start + 1);
}

void write_vehicle_mods(FILE* file, const VehicleMods& mods) {
        bool hasExtras = !mods.extrasOn.empty();
        bool hasColors = mods.primaryColor >= 0 || mods.secondaryColor >= 0 || mods.pearlescent >= 0 || mods.wheelColor >= 0;
        bool hasNeon = std::any_of(std::begin(mods.neon), std::end(mods.neon), [](bool value) { return value; });
        if (!hasExtras && !hasColors && !hasNeon) {
                return;
        }
        std::fprintf(file,
                     "    <VehicleMods primaryColor=\"%d\" secondaryColor=\"%d\" pearlescent=\"%d\" wheelColor=\"%d\" neon0=\"%d\" neon1=\"%d\" neon2=\"%d\" neon3=\"%d\">\n",
                     mods.primaryColor,
                     mods.secondaryColor,
                     mods.pearlescent,
                     mods.wheelColor,
                     mods.neon[0] ? 1 : 0,
                     mods.neon[1] ? 1 : 0,
                     mods.neon[2] ? 1 : 0,
                     mods.neon[3] ? 1 : 0);
        std::fprintf(file, "      <Extras>");
        for (size_t i = 0; i < mods.extrasOn.size(); ++i) {
                std::fprintf(file, "%s%d", i == 0 ? "" : " ", mods.extrasOn[i]);
        }
        std::fprintf(file, "</Extras>\n");
        std::fprintf(file, "    </VehicleMods>\n");
}

VehicleMods parse_vehicle_mods(const std::string& block) {
        VehicleMods mods;
        std::string segment = get_tag_segment(block, "VehicleMods");
        if (segment.empty()) {
                return mods;
        }
        mods.primaryColor = parse_int(get_attribute(segment, "primaryColor"));
        mods.secondaryColor = parse_int(get_attribute(segment, "secondaryColor"));
        mods.pearlescent = parse_int(get_attribute(segment, "pearlescent"));
        mods.wheelColor = parse_int(get_attribute(segment, "wheelColor"));
        mods.neon[0] = parse_bool(get_attribute(segment, "neon0"));
        mods.neon[1] = parse_bool(get_attribute(segment, "neon1"));
        mods.neon[2] = parse_bool(get_attribute(segment, "neon2"));
        mods.neon[3] = parse_bool(get_attribute(segment, "neon3"));
        std::string extrasContent = get_tag_content(block, "Extras");
        if (!extrasContent.empty()) {
                std::istringstream stream(extrasContent);
                int extra = 0;
                while (stream >> extra) {
                        mods.extrasOn.push_back(extra);
                }
        }
        return mods;
}

} // namespace

namespace serializer {

bool save_map(const char* path, const std::vector<spooner_graph::EntityRecord>& entities) {
        if (path == nullptr) {
                return false;
        }
        FILE* file = std::fopen(path, "wb");
        if (!file) {
                return false;
        }
        std::fprintf(file, "<SpoonerMap>\n");
        for (const auto& entity : entities) {
                std::fprintf(file, "  <Entity id=\"%d\" type=\"%s\">\n", entity.id, type_to_string(entity.type).c_str());
                std::fprintf(file, "    <ModelHash>0x%08X</ModelHash>\n", entity.model);
                std::fprintf(file,
                             "    <PositionRotation x=\"%.4f\" y=\"%.4f\" z=\"%.4f\" pitch=\"%.4f\" roll=\"%.4f\" yaw=\"%.4f\"/>\n",
                             entity.transform.pos.x,
                             entity.transform.pos.y,
                             entity.transform.pos.z,
                             entity.transform.rot.x,
                             entity.transform.rot.y,
                             entity.transform.rot.z);
                std::fprintf(file,
                             "    <Props visible=\"%d\" collision=\"%d\" invincible=\"%d\" frozen=\"%d\" alpha=\"%d\"/>\n",
                             entity.props.visible ? 1 : 0,
                             entity.props.collision ? 1 : 0,
                             entity.props.invincible ? 1 : 0,
                             entity.props.frozen ? 1 : 0,
                             entity.props.alpha);
                if (entity.attachment.attached && entity.attachment.parent != 0) {
                        const AttachSpec& spec = entity.attachment.spec;
                        std::fprintf(file,
                                     "    <Attachment parentId=\"%d\" bone=\"%s\" offx=\"%.4f\" offy=\"%.4f\" offz=\"%.4f\" rotx=\"%.4f\" roty=\"%.4f\" rotz=\"%.4f\" collisions=\"%d\" softPin=\"%d\"/>\n",
                                     entity.attachment.parent,
                                     spec.bone.c_str(),
                                     spec.offset.x,
                                     spec.offset.y,
                                     spec.offset.z,
                                     spec.rot.x,
                                     spec.rot.y,
                                     spec.rot.z,
                                     spec.collisions ? 1 : 0,
                                     spec.softPin ? 1 : 0);
                }
                write_vehicle_mods(file, entity.vehicleMods);
                std::fprintf(file, "  </Entity>\n");
        }
        std::fprintf(file, "</SpoonerMap>\n");
        std::fclose(file);
        return true;
}

bool load_map(const char* path, std::vector<LoadedEntity>& entities) {
        entities.clear();
        if (path == nullptr) {
                return false;
        }
        FILE* file = std::fopen(path, "rb");
        if (!file) {
                return false;
        }
        std::fseek(file, 0, SEEK_END);
        long size = std::ftell(file);
        if (size <= 0) {
                std::fclose(file);
                return false;
        }
        std::fseek(file, 0, SEEK_SET);
        std::string buffer;
        buffer.resize(static_cast<size_t>(size));
        if (std::fread(&buffer[0], 1, static_cast<size_t>(size), file) != static_cast<size_t>(size)) {
                std::fclose(file);
                return false;
        }
        std::fclose(file);

        size_t pos = 0;
        while (true) {
                size_t entityPos = buffer.find("<Entity", pos);
                if (entityPos == std::string::npos) {
                        break;
                }
                size_t headerEnd = buffer.find('>', entityPos);
                if (headerEnd == std::string::npos) {
                        break;
                }
                std::string header = buffer.substr(entityPos, headerEnd - entityPos + 1);
                size_t closePos = buffer.find("</Entity>", headerEnd);
                if (closePos == std::string::npos) {
                        break;
                }
                std::string block = buffer.substr(headerEnd + 1, closePos - headerEnd - 1);

                LoadedEntity entry;
                entry.legacyId = parse_int(get_attribute(header, "id"));
                entry.type = type_from_string(get_attribute(header, "type"));
                entry.model = parse_hash(get_tag_content(block, "ModelHash"));

                std::string transformSegment = get_tag_segment(block, "PositionRotation");
                if (!transformSegment.empty()) {
                        entry.transform.pos.x = parse_float(get_attribute(transformSegment, "x"));
                        entry.transform.pos.y = parse_float(get_attribute(transformSegment, "y"));
                        entry.transform.pos.z = parse_float(get_attribute(transformSegment, "z"));
                        entry.transform.rot.x = parse_float(get_attribute(transformSegment, "pitch"));
                        entry.transform.rot.y = parse_float(get_attribute(transformSegment, "roll"));
                        entry.transform.rot.z = parse_float(get_attribute(transformSegment, "yaw"));
                }

                std::string propsSegment = get_tag_segment(block, "Props");
                if (!propsSegment.empty()) {
                        entry.props.visible = parse_bool(get_attribute(propsSegment, "visible"));
                        entry.props.collision = parse_bool(get_attribute(propsSegment, "collision"));
                        entry.props.invincible = parse_bool(get_attribute(propsSegment, "invincible"));
                        entry.props.frozen = parse_bool(get_attribute(propsSegment, "frozen"));
                        entry.props.alpha = parse_int(get_attribute(propsSegment, "alpha"));
                }

                std::string attachmentSegment = get_tag_segment(block, "Attachment");
                if (!attachmentSegment.empty()) {
                        entry.attachment.attached = true;
                        entry.attachment.parent = parse_int(get_attribute(attachmentSegment, "parentId"));
                        entry.attachment.spec.bone = get_attribute(attachmentSegment, "bone");
                        entry.attachment.spec.offset.x = parse_float(get_attribute(attachmentSegment, "offx"));
                        entry.attachment.spec.offset.y = parse_float(get_attribute(attachmentSegment, "offy"));
                        entry.attachment.spec.offset.z = parse_float(get_attribute(attachmentSegment, "offz"));
                        entry.attachment.spec.rot.x = parse_float(get_attribute(attachmentSegment, "rotx"));
                        entry.attachment.spec.rot.y = parse_float(get_attribute(attachmentSegment, "roty"));
                        entry.attachment.spec.rot.z = parse_float(get_attribute(attachmentSegment, "rotz"));
                        entry.attachment.spec.collisions = parse_bool(get_attribute(attachmentSegment, "collisions"));
                        entry.attachment.spec.softPin = parse_bool(get_attribute(attachmentSegment, "softPin"));
                        if (entry.attachment.parent == 0) {
                                entry.attachment.attached = false;
                        }
                }

                entry.vehicleMods = parse_vehicle_mods(block);
                entities.push_back(entry);
                pos = closePos + std::strlen("</Entity>");
        }

        return !entities.empty();
}

} // namespace serializer
} // namespace menyoo
