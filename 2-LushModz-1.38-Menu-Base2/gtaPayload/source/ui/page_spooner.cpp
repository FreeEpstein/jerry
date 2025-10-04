#include "ui/page_spooner.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "Functions.h"
#include "MenuUtils.h"
#include "Enums.h"
#include "natives.h"

namespace menyoo {
namespace ui {
namespace {

std::vector<EntityId> g_entities;
int g_selectedIndex = -1;
EntityId g_selectedId = 0;
EntityId g_lastSelection = 0;
EntityProps g_selectedProps{};
Transform g_selectedTransform{};
AttachmentInfo g_selectedAttachment{};
AttachSpec g_attachSpec{};
EntityId g_attachParent = 0;
std::string g_modelInput = "0xB779A091";
std::string g_mapName = "spooner.xml";
uint32_t g_lastMoveTick = 0;

const float kMoveStep = 0.25f;
const float kRotStep = 5.0f;
const uint32_t kMoveIntervalMs = 120;

const char* TypeName(EntityType type) {
        switch (type) {
        case EntityType::Vehicle:
                return "Vehicle";
        case EntityType::Ped:
                return "Ped";
        case EntityType::Object:
        default:
                return "Object";
        }
}

Hash ParseModel(const std::string& input) {
        if (input.empty()) {
                return 0;
        }
        bool isHex = input.size() > 2 && input[0] == '0' && (input[1] == 'x' || input[1] == 'X');
        auto isDecimal = [](const std::string& value) {
                if (value.empty()) {
                        return false;
                }
                for (char ch : value) {
                        if (ch < '0' || ch > '9') {
                                return false;
                        }
                }
                return true;
        };
        bool numeric = isHex;
        if (!isHex) {
                numeric = isDecimal(input);
        }
        if (isHex || numeric) {
                int base = isHex ? 16 : 10;
                return static_cast<Hash>(std::strtoul(input.c_str(), nullptr, base));
        }
        return GET_HASH_KEY(input.c_str());
}

void ToLowerInPlace(std::string& value) {
        for (char& ch : value) {
                if (ch >= 'A' && ch <= 'Z') {
                        ch = static_cast<char>(ch - 'A' + 'a');
                }
        }
}

bool PromptKeyboard(const char* title, std::string& value, size_t maxLen) {
        DISPLAY_ONSCREEN_KEYBOARD(1, title, "", value.c_str(), "", "", "", static_cast<int>(maxLen));
        while (true) {
                int status = UPDATE_ONSCREEN_KEYBOARD();
                if (status == 0) {
                        WAIT(0);
                        continue;
                }
                if (status == 1) {
                        const char* result = GET_ONSCREEN_KEYBOARD_RESULT();
                        if (result != nullptr) {
                                value = result;
                        }
                        return true;
                }
                return false;
        }
}

void UpdateSelectionState(bool forceAttachment) {
        if (g_selectedId == 0) {
                g_selectedProps = EntityProps{};
                g_selectedTransform = Transform{};
                g_selectedAttachment = AttachmentInfo{};
                return;
        }
        g_selectedProps = get_entity_props(g_selectedId);
        g_selectedTransform = get_entity_transform(g_selectedId);
        g_selectedAttachment = get_attachment(g_selectedId);
        if (forceAttachment || g_lastSelection != g_selectedId) {
                if (g_selectedAttachment.attached) {
                        g_attachParent = g_selectedAttachment.parent;
                        g_attachSpec = g_selectedAttachment.spec;
                } else {
                        g_attachParent = 0;
                        g_attachSpec = AttachSpec{};
                }
        }
        g_lastSelection = g_selectedId;
}

void RefreshEntities(bool forceState) {
        EntityId previousId = g_selectedId;
        g_entities = list_entities();
        if (g_entities.empty()) {
                g_selectedIndex = -1;
                g_selectedId = 0;
                UpdateSelectionState(true);
                return;
        }
        if (previousId != 0) {
                auto it = std::find(g_entities.begin(), g_entities.end(), previousId);
                if (it != g_entities.end()) {
                        g_selectedIndex = static_cast<int>(std::distance(g_entities.begin(), it));
                }
        }
        if (g_selectedIndex < 0 || g_selectedIndex >= static_cast<int>(g_entities.size())) {
                g_selectedIndex = 0;
        }
        g_selectedId = g_entities[g_selectedIndex];
        UpdateSelectionState(forceState || previousId != g_selectedId);
}

void SpawnEntityOfType(EntityType type) {
        Hash model = ParseModel(g_modelInput);
        if (model == 0) {
                print("~r~Invalid model hash");
                return;
        }
        vector3 playerPos = GET_ENTITY_COORDS(PLAYER_PED_ID(), false);
        float heading = GET_ENTITY_HEADING(PLAYER_PED_ID());
        Transform transform{};
        transform.pos = {playerPos.x, playerPos.y, playerPos.z};
        transform.rot = {0.0f, 0.0f, heading};
        EntityId newId = spawn_entity(type, model, transform);
        if (newId == 0) {
                print("~r~Spawn failed");
                return;
        }
        g_selectedId = newId;
        g_selectedIndex = -1;
        RefreshEntities(true);
}

void CloneSelected() {
        if (g_selectedId == 0) {
                return;
        }
        EntityType type = entity_type(g_selectedId);
        Hash model = entity_model(g_selectedId);
        if (model == 0) {
                return;
        }
        Transform transform = g_selectedTransform;
        transform.pos.y += 2.0f;
        EntityId newId = spawn_entity(type, model, transform);
        if (newId == 0) {
                return;
        }
        set_entity_props(newId, g_selectedProps);
        g_selectedId = newId;
        g_selectedIndex = -1;
        RefreshEntities(true);
}

void DeleteSelected() {
        if (g_selectedId == 0) {
                return;
        }
        if (delete_entity(g_selectedId)) {
                g_selectedId = 0;
                g_selectedIndex = -1;
                RefreshEntities(true);
        }
}

std::vector<EntityId> ParentChoices() {
        std::vector<EntityId> choices;
        choices.push_back(0);
        for (EntityId id : g_entities) {
                if (id != g_selectedId) {
                        choices.push_back(id);
                }
        }
        return choices;
}

void CycleParent(int direction) {
        std::vector<EntityId> choices = ParentChoices();
        if (choices.empty()) {
                g_attachParent = 0;
                return;
        }
        auto it = std::find(choices.begin(), choices.end(), g_attachParent);
        int index = 0;
        if (it != choices.end()) {
                index = static_cast<int>(std::distance(choices.begin(), it));
        }
        int count = static_cast<int>(choices.size());
        index = (index + direction + count) % count;
        g_attachParent = choices[index];
}

void ApplyAttachment() {
        if (g_selectedId == 0 || g_attachParent == 0 || g_attachParent == g_selectedId) {
                return;
        }
        attach_entity(g_selectedId, g_attachParent, g_attachSpec);
        UpdateSelectionState(true);
}

void DetachSelected() {
        if (g_selectedId == 0) {
                return;
        }
        detach_entity(g_selectedId);
        g_attachParent = 0;
        g_attachSpec = AttachSpec{};
        UpdateSelectionState(true);
}

void HandleSelectionCycling(int optionIndex) {
        if (currentOption != optionIndex || g_entities.empty()) {
                return;
        }
        bool changed = false;
        if (leftPress) {
                g_selectedIndex--;
                if (g_selectedIndex < 0) {
                        g_selectedIndex = static_cast<int>(g_entities.size()) - 1;
                }
                leftPress = false;
                changed = true;
        }
        if (rightPress) {
                g_selectedIndex++;
                if (g_selectedIndex >= static_cast<int>(g_entities.size())) {
                        g_selectedIndex = 0;
                }
                rightPress = false;
                changed = true;
        }
        if (changed) {
                g_selectedId = g_entities[g_selectedIndex];
                UpdateSelectionState(true);
        }
}

void HandleParentCycling(int optionIndex) {
        if (currentOption != optionIndex) {
                return;
        }
        if (leftPress) {
                        CycleParent(-1);
                        leftPress = false;
        }
        if (rightPress) {
                        CycleParent(1);
                        rightPress = false;
        }
}

} // namespace

void DrawSpoonerMenu() {
        RefreshEntities(false);
        subTitle("Spooner");

        char buffer[128];
        std::snprintf(buffer, sizeof(buffer), "Model Hash: %s", g_modelInput.c_str());
        addOption(buffer);
        int modelOption = optionCount;

        addOption("Spawn Vehicle");
        int spawnVehicleOption = optionCount;

        addOption("Spawn Ped");
        int spawnPedOption = optionCount;

        addOption("Spawn Object");
        int spawnObjectOption = optionCount;

        const char* selectedType = g_selectedId != 0 ? TypeName(entity_type(g_selectedId)) : "None";
        if (g_selectedId != 0) {
                std::snprintf(buffer, sizeof(buffer), "Selected: %s %d", selectedType, g_selectedId);
        } else {
                std::snprintf(buffer, sizeof(buffer), "Selected: <none>");
        }
        addOption(buffer);
        int selectOption = optionCount;

        addOption("Clone Selected");
        int cloneOption = optionCount;

        addOption("Delete Selected");
        int deleteOption = optionCount;

        int freezeOption = -1;
        int invincibleOption = -1;
        int alphaOption = -1;
        int parentOption = -1;
        int boneOption = -1;
        int collisionsOption = -1;
        int softPinOption = -1;
        int applyAttachmentOption = -1;
        int detachOption = -1;

        int mapNameOption = -1;
        int saveOption = -1;
        int loadOption = -1;

        if (g_selectedId != 0) {
                CheckBox("Freeze", g_selectedProps.frozen);
                freezeOption = optionCount;
                CheckBox("Invincible", g_selectedProps.invincible);
                invincibleOption = optionCount;
                int prevAlpha = g_selectedProps.alpha;
                addIntOption("Alpha", &g_selectedProps.alpha, 0, 255, false);
                alphaOption = optionCount;
                if (prevAlpha != g_selectedProps.alpha) {
                        set_entity_props(g_selectedId, g_selectedProps);
                }

                char parentLabel[64];
                if (g_attachParent == 0) {
                        std::snprintf(parentLabel, sizeof(parentLabel), "Attachment Parent: None");
                } else {
                        std::snprintf(parentLabel, sizeof(parentLabel), "Attachment Parent: %d", g_attachParent);
                }
                addOption(parentLabel);
                parentOption = optionCount;

                std::snprintf(buffer, sizeof(buffer), "Attachment Bone: %s", g_attachSpec.bone.empty() ? "<none>" : g_attachSpec.bone.c_str());
                addOption(buffer);
                boneOption = optionCount;

                addFloatOption("Offset X", &g_attachSpec.offset.x, -50.0f, 50.0f, false);
                addFloatOption("Offset Y", &g_attachSpec.offset.y, -50.0f, 50.0f, false);
                addFloatOption("Offset Z", &g_attachSpec.offset.z, -50.0f, 50.0f, false);

                addFloatOption("Rotation X", &g_attachSpec.rot.x, -360.0f, 360.0f, false);
                addFloatOption("Rotation Y", &g_attachSpec.rot.y, -360.0f, 360.0f, false);
                addFloatOption("Rotation Z", &g_attachSpec.rot.z, -360.0f, 360.0f, false);

                CheckBox("Attachment Collisions", g_attachSpec.collisions);
                collisionsOption = optionCount;
                CheckBox("Attachment Soft Pin", g_attachSpec.softPin);
                softPinOption = optionCount;

                addOption("Apply Attachment");
                applyAttachmentOption = optionCount;

                addOption("Detach");
                detachOption = optionCount;
        }

        std::snprintf(buffer, sizeof(buffer), "Map File: %s", g_mapName.c_str());
        addOption(buffer);
        mapNameOption = optionCount;

        addOption("Save Map");
        saveOption = optionCount;

        addOption("Load Map");
        loadOption = optionCount;

        HandleSelectionCycling(selectOption);
        if (parentOption != -1) {
                HandleParentCycling(parentOption);
        }

        int pressed = GET();
        if (pressed == modelOption) {
                if (PromptKeyboard("MODEL", g_modelInput, 64)) {
                        ToLowerInPlace(g_modelInput);
                }
        } else if (pressed == spawnVehicleOption) {
                SpawnEntityOfType(EntityType::Vehicle);
        } else if (pressed == spawnPedOption) {
                SpawnEntityOfType(EntityType::Ped);
        } else if (pressed == spawnObjectOption) {
                SpawnEntityOfType(EntityType::Object);
        } else if (pressed == cloneOption) {
                CloneSelected();
        } else if (pressed == deleteOption) {
                DeleteSelected();
        } else if (pressed == freezeOption) {
                g_selectedProps.frozen = !g_selectedProps.frozen;
                set_entity_props(g_selectedId, g_selectedProps);
        } else if (pressed == invincibleOption) {
                g_selectedProps.invincible = !g_selectedProps.invincible;
                set_entity_props(g_selectedId, g_selectedProps);
        } else if (pressed == boneOption) {
                PromptKeyboard("BONE", g_attachSpec.bone, 32);
        } else if (pressed == collisionsOption) {
                g_attachSpec.collisions = !g_attachSpec.collisions;
        } else if (pressed == softPinOption) {
                g_attachSpec.softPin = !g_attachSpec.softPin;
        } else if (pressed == applyAttachmentOption) {
                ApplyAttachment();
        } else if (pressed == detachOption) {
                DetachSelected();
        } else if (pressed == mapNameOption) {
                PromptKeyboard("MAP", g_mapName, 64);
        } else if (pressed == saveOption) {
                if (save_map(g_mapName.c_str())) {
                        print("~g~Map saved");
                } else {
                        print("~r~Save failed");
                }
        } else if (pressed == loadOption) {
                if (load_map(g_mapName.c_str())) {
                        print("~g~Map loaded");
                        RefreshEntities(true);
                } else {
                        print("~r~Load failed");
                }
        }
}

bool HandleSpoonerControls() {
        if (g_selectedId == 0) {
                return false;
        }
        bool up = IS_DISABLED_CONTROL_PRESSED(0, INPUT_SCRIPT_PAD_UP);
        bool down = IS_DISABLED_CONTROL_PRESSED(0, INPUT_SCRIPT_PAD_DOWN);
        bool left = IS_DISABLED_CONTROL_PRESSED(0, INPUT_SCRIPT_PAD_LEFT);
        bool right = IS_DISABLED_CONTROL_PRESSED(0, INPUT_SCRIPT_PAD_RIGHT);
        if (!up && !down && !left && !right) {
                return false;
        }
        bool l1 = IS_DISABLED_CONTROL_PRESSED(0, INPUT_FRONTEND_LB);
        bool r1 = IS_DISABLED_CONTROL_PRESSED(0, INPUT_FRONTEND_RB);
        uint32_t now = GET_GAME_TIMER();
        if (now - g_lastMoveTick < kMoveIntervalMs) {
                return true;
        }
        g_lastMoveTick = now;
        Transform transform = get_entity_transform(g_selectedId);
        bool moved = false;
        if (l1) {
                if (left) {
                        transform.rot.z -= kRotStep;
                        moved = true;
                }
                if (right) {
                        transform.rot.z += kRotStep;
                        moved = true;
                }
                if (up) {
                        transform.rot.x += kRotStep;
                        moved = true;
                }
                if (down) {
                        transform.rot.x -= kRotStep;
                        moved = true;
                }
        } else if (r1) {
                if (left) {
                        transform.rot.y -= kRotStep;
                        moved = true;
                }
                if (right) {
                        transform.rot.y += kRotStep;
                        moved = true;
                }
        } else {
                if (up) {
                        transform.pos.z += kMoveStep;
                        moved = true;
                }
                if (down) {
                        transform.pos.z -= kMoveStep;
                        moved = true;
                }
                if (left) {
                        transform.pos.x -= kMoveStep;
                        moved = true;
                }
                if (right) {
                        transform.pos.x += kMoveStep;
                        moved = true;
                }
        }
        if (moved) {
                set_entity_transform(g_selectedId, transform);
                g_selectedTransform = transform;
        }
        return moved;
}

} // namespace ui
} // namespace menyoo
