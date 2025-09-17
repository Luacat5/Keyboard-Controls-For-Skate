#include "mapping.h"

static const std::unordered_map<std::string, int> vkLookup = {
    {"VK_BACK", VK_BACK},
    {"VK_TAB", VK_TAB},
    {"VK_RETURN", VK_RETURN},
    {"VK_SHIFT", VK_SHIFT},
    {"VK_CONTROL", VK_CONTROL},
    {"VK_MENU", VK_MENU}, // Alt
    {"VK_PAUSE", VK_PAUSE},
    {"VK_CAPITAL", VK_CAPITAL},
    {"VK_ESCAPE", VK_ESCAPE},
    {"VK_SPACE", VK_SPACE},
    {"VK_PRIOR", VK_PRIOR},   // Page Up
    {"VK_NEXT", VK_NEXT},     // Page Down
    {"VK_END", VK_END},
    {"VK_HOME", VK_HOME},
    {"VK_LEFT", VK_LEFT},
    {"VK_UP", VK_UP},
    {"VK_RIGHT", VK_RIGHT},
    {"VK_DOWN", VK_DOWN},
    {"VK_INSERT", VK_INSERT},
    {"VK_DELETE", VK_DELETE},
    {"VK_LSHIFT", VK_LSHIFT},
    {"VK_RSHIFT", VK_RSHIFT},
    {"VK_LCONTROL", VK_LCONTROL},
    {"VK_RCONTROL", VK_RCONTROL},
    {"VK_LMENU", VK_LMENU},   // Left Alt
    {"VK_RMENU", VK_RMENU},   // Right Alt
    {"VK_F1", VK_F1}, {"VK_F2", VK_F2}, {"VK_F3", VK_F3}, {"VK_F4", VK_F4},
    {"VK_F5", VK_F5}, {"VK_F6", VK_F6}, {"VK_F7", VK_F7}, {"VK_F8", VK_F8},
    {"VK_F9", VK_F9}, {"VK_F10", VK_F10}, {"VK_F11", VK_F11}, {"VK_F12", VK_F12},
    {"VK_NUMPAD0", VK_NUMPAD0},
    {"VK_NUMPAD1", VK_NUMPAD1},
    {"VK_NUMPAD2", VK_NUMPAD2},
    {"VK_NUMPAD3", VK_NUMPAD3},
    {"VK_NUMPAD4", VK_NUMPAD4},
    {"VK_NUMPAD5", VK_NUMPAD5},
    {"VK_NUMPAD6", VK_NUMPAD6},
    {"VK_NUMPAD7", VK_NUMPAD7},
    {"VK_NUMPAD8", VK_NUMPAD8},
    {"VK_NUMPAD9", VK_NUMPAD9}
}; 
// literally no library for this that I know of

int GetRepresentative(const std::string& inputStr) {
    if (inputStr.size() > 1) {
        auto it = vkLookup.find(inputStr);
        if (it != vkLookup.end()) {
            return it->second;
        } else {
            std::cerr << "Fatal error: unknown VK mapping: " << inputStr << std::endl;
            return -1;
        }
    }

    return static_cast<int>(inputStr[0]);
}


const std::string stickNames[4] = {"up","down","left","right"};
const std::string allButtons[] = {
    "Square", "X", "Circle", "Triangle",
    "L1", "R1", "L2", "R2", "L3", "R3",
    "PAUSE", "SELECT"
};

void bindStick(const std::string& stickName, std::unordered_map<int, std::string>& map) {
    const auto& StickInfo = config[stickName];

    for (int i = 0; i < 4; i++) {
        const std::string& DirectionName = stickNames[i];
        map.insert_or_assign(
            GetRepresentative(StickInfo[DirectionName].get<std::string>()), stickName + "_" + DirectionName);
    }
}


static std::unordered_map<int, std::string> KeyToAction;
static std::unordered_map<std::string, bool> IsKeyDown;


std::unordered_map<int, std::string> MakeKeyMap() {

    bindStick("left_stick", KeyToAction);
    bindStick("right_stick", KeyToAction);
    bindStick("dpad", KeyToAction);

    for (int i = 0; i < (sizeof(allButtons) / sizeof(allButtons[0])); i++) {
        const auto& buttonName = allButtons[i];
        auto vkCode = GetRepresentative(config["buttons"][buttonName].get<std::string>());
        KeyToAction.insert_or_assign(vkCode, buttonName);


        std::cout << "assigned " << vkCode << " to " << KeyToAction[vkCode] << std::endl;

    }

    KeyToAction.insert_or_assign( 
        GetRepresentative(config["special_keys"]["right_stick_multiplier_key"].get<std::string>()), "right_stick_multiplier");

    KeyToAction.insert_or_assign( 
        GetRepresentative(config["special_keys"]["EXIT"].get<std::string>()), "EXIT");

    return KeyToAction;
}




bool processKeyEvent(int vkCode, WPARAM edge){
    auto it = KeyToAction.find(vkCode);
    if (it == KeyToAction.end()) {
        std::cout << "Not a known action: " << vkCode << std::endl;
        return false;
    }
    std::string actionName = it->second;

    std::cout << "Found: " << vkCode <<" assigned to " << actionName << std::endl;;

    if (edge == WM_KEYDOWN || edge == WM_SYSKEYDOWN) {
        IsKeyDown[actionName] = true;
    } else if (edge == WM_KEYUP || edge == WM_SYSKEYUP) {
        IsKeyDown[actionName] = false;
    }

    return true;
}


bool GetKeyDown(std::string Name){
    return IsKeyDown[Name];
}