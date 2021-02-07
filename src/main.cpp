#include "main.hpp"
#include "SettingsViewController.hpp"
using namespace NoMissText;

#include <string>
using namespace std;

#include "GlobalNamespace/FlyingSpriteSpawner.hpp"
#include "GlobalNamespace/ComboUIController.hpp"
#include "GlobalNamespace/GameEnergyUIPanel.hpp"

#include "GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/StandardLevelFailedController.hpp"
#include "GlobalNamespace/IDifficultyBeatmap.hpp"
#include "GlobalNamespace/OverrideEnvironmentSettings.hpp"
#include "GlobalNamespace/ColorScheme.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include "GlobalNamespace/PlayerSpecificSettings.hpp"
#include "GlobalNamespace/PracticeSettings.hpp"
#include "GlobalNamespace/MultiplayerLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/IPreviewBeatmapLevel.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/MissionLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/MissionObjective.hpp"
using namespace GlobalNamespace;

#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Playables/PlayableDirector.hpp"
#include "UnityEngine/UI/Image.hpp"
using namespace UnityEngine;

#include "custom-types/shared/register.hpp"
#include "questui/shared/QuestUI.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

static ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

static bool hideText;
static bool hideBreakLines;
static bool subtleNoFail;

static bool noFailEnabled = false;

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
    subtleNoFail = getConfig().config["subtleNoFail"].GetBool();
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

MAKE_HOOK_OFFSETLESS(DepleteEnergy, void, GameEnergyUIPanel* self, float energy) {
    DepleteEnergy(self, energy);
    if (subtleNoFail && noFailEnabled) {
        if (energy <= 1E-05f) {
            self->playableDirector->Stop(); // stop the WOOOSH
            self->playableDirector->get_gameObject()->SetActive(false); // hide the WOOOSH
            self->energyBar->set_enabled(true); // re-enable the energy bar (tho the game still hides it)
        }
    }
}

// fine bois who get if ur using no fail or not
MAKE_HOOK_OFFSETLESS(LevelStartStandard, void, StandardLevelScenesTransitionSetupDataSO* self, string* gameMode, IDifficultyBeatmap* difficultyBeatmap, OverrideEnvironmentSettings* overrideEnvironmentSettings, ColorScheme* overrideColorScheme, GameplayModifiers* gameplayModifiers, PlayerSpecificSettings* playerSpecificSettings, PracticeSettings* practiceSettings, string* backButtonText, bool* useTestNoteCutSoundEffects) {
    LevelStartStandard(self, gameMode, difficultyBeatmap, overrideEnvironmentSettings, overrideColorScheme, gameplayModifiers, playerSpecificSettings, practiceSettings, backButtonText, useTestNoteCutSoundEffects);
    noFailEnabled = gameplayModifiers->noFailOn0Energy;
}

MAKE_HOOK_OFFSETLESS(LevelStartMultiplayer, void, MultiplayerLevelScenesTransitionSetupDataSO* self, string* gameMode, IPreviewBeatmapLevel* previewBeatmapLevel, BeatmapDifficulty* beatmapDifficulty, BeatmapCharacteristicSO* beatmapCharacteristic, IDifficultyBeatmap* difficultyBeatmap, ColorScheme* overrideColorScheme, GameplayModifiers* gameplayModifiers, PlayerSpecificSettings* playerSpecificSettings, PracticeSettings* practiceSettings, bool* useTestNoteCutSoundEffects) {
    LevelStartMultiplayer(self, gameMode, previewBeatmapLevel, beatmapDifficulty, beatmapCharacteristic, difficultyBeatmap, overrideColorScheme, gameplayModifiers, playerSpecificSettings, practiceSettings, useTestNoteCutSoundEffects);
    noFailEnabled = gameplayModifiers->noFailOn0Energy;
}

MAKE_HOOK_OFFSETLESS(LevelStartMission, void, MissionLevelScenesTransitionSetupDataSO* self, string* missionId, IDifficultyBeatmap* difficultyBeatmap, Array<MissionObjective*>* missionObjectives, ColorScheme* overrideColorScheme, GameplayModifiers* gameplayModifiers, PlayerSpecificSettings* playerSpecificSettings, string* backButtonText) {
    LevelStartMission(self, missionId, difficultyBeatmap, missionObjectives, overrideColorScheme, gameplayModifiers, playerSpecificSettings, backButtonText);
    noFailEnabled = gameplayModifiers->noFailOn0Energy;
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
    if (!getConfig().config.HasMember("subtleNoFail")) {
        getConfig().config.AddMember("subtleNoFail", rapidjson::Value(0).SetBool(true), allocator);
        getConfig().Write();
        getLogger().info("Config written. (added subtleNoFail)");
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
    INSTALL_HOOK_OFFSETLESS(getLogger(), DepleteEnergy, il2cpp_utils::FindMethodUnsafe("", "GameEnergyUIPanel", "RefreshEnergyUI", 1));

    INSTALL_HOOK_OFFSETLESS(getLogger(), LevelStartStandard, il2cpp_utils::FindMethodUnsafe("", "StandardLevelScenesTransitionSetupDataSO", "Init", 9));
    INSTALL_HOOK_OFFSETLESS(getLogger(), LevelStartMultiplayer, il2cpp_utils::FindMethodUnsafe("", "MultiplayerLevelScenesTransitionSetupDataSO", "Init", 10));
    INSTALL_HOOK_OFFSETLESS(getLogger(), LevelStartMission, il2cpp_utils::FindMethodUnsafe("", "MissionLevelScenesTransitionSetupDataSO", "Init", 7));

    getLogger().info("Installed all hooks!");
}