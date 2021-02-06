#include "main.hpp"
#include "SettingsViewController.hpp"
using namespace NoMissText;

#include "GlobalNamespace/FlyingSpriteSpawner.hpp"
#include "GlobalNamespace/ComboUIController.hpp"
using namespace GlobalNamespace;

#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"
using namespace UnityEngine;

#include "custom-types/shared/register.hpp"
#include "questui/shared/QuestUI.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

static ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

static bool hideText;
static bool hideBreakLines;

// Loads the config from disk using our modInfo, then returns it for use
Configuration& getConfig() {
    static Configuration config(modInfo);
    config.Load();
    return config;
}

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

void updateVals() {
    hideText = getConfig().config["hideText"].GetBool();
    hideBreakLines = getConfig().config["hideBreakLines"].GetBool();
    getLogger().info("Updated configuration values!");
}

MAKE_HOOK_OFFSETLESS(StopFlyin, void, FlyingSpriteSpawner* self, UnityEngine::Vector3 pos, UnityEngine::Quaternion rotation, UnityEngine::Quaternion inverseRotation) {
    if (!hideText) {
        StopFlyin(self, pos, rotation, inverseRotation);
    }
}

MAKE_HOOK_OFFSETLESS(ComboBreak, void, ComboUIController* self) {
    if (hideBreakLines) {
        if (!self->comboLost) {
            self->comboLost = true;
            self->get_transform()->Find(il2cpp_utils::createcsstr("Line0"))->get_gameObject()->SetActive(false);
            self->get_transform()->Find(il2cpp_utils::createcsstr("Line1"))->get_gameObject()->SetActive(false);
        }
    } else {
        ComboBreak(self);
    }
}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = ID;
    info.version = VERSION;
    modInfo = info;
	
    getConfig().Load(); // Load the config file

    rapidjson::Document::AllocatorType& allocator = getConfig().config.GetAllocator();

    if (!getConfig().config.HasMember("hideText")) {
        getConfig().config.AddMember("hideText", rapidjson::Value(0).SetBool(true), allocator);
        getConfig().Write();
        getLogger().info("Config written. (added hideText)");
    }
    if (!getConfig().config.HasMember("hideBreakLines")) {
        getConfig().config.AddMember("hideBreakLines", rapidjson::Value(0).SetBool(true), allocator);
        getConfig().Write();
        getLogger().info("Config written. (added hideBreakLines)");
    }
    updateVals();

    getLogger().info("Completed setup!");
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();
    QuestUI::Init();

    getLogger().info("Installing hooks...");

    custom_types::Register::RegisterType<SettingsViewController>();
    QuestUI::Register::RegisterModSettingsViewController<SettingsViewController*>(modInfo);
    
    INSTALL_HOOK_OFFSETLESS(getLogger(), StopFlyin, il2cpp_utils::FindMethodUnsafe("", "FlyingSpriteSpawner", "SpawnFlyingSprite", 3));
    INSTALL_HOOK_OFFSETLESS(getLogger(), ComboBreak, il2cpp_utils::FindMethodUnsafe("", "ComboUIController", "HandleComboBreakingEventHappened", 0));

    getLogger().info("Installed all hooks!");
}