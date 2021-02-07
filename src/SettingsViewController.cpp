#include "SettingsViewController.hpp"
#include "main.hpp"
using namespace NoMissText;

#include "questui/shared/BeatSaberUI.hpp"
using namespace QuestUI;

#include "UnityEngine/UI/VerticalLayoutGroup.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"
#include "UnityEngine/Events/UnityAction_1.hpp"
using namespace UnityEngine::UI;
using namespace UnityEngine::Events;

DEFINE_CLASS(SettingsViewController);

void onChangeMissText(SettingsViewController* self, bool newValue) {
    getConfig().config["hideText"].SetBool(newValue);
}

void onChangeComboBreak(SettingsViewController* self, bool newValue) {
    getConfig().config["hideBreakLines"].SetBool(newValue);
}

void onChangeSubtleNoFail(SettingsViewController* self, bool newValue) {
    getConfig().config["subtleNoFail"].SetBool(newValue);
}

void SettingsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (!(firstActivation && addedToHierarchy)) return;

    VerticalLayoutGroup* layout = BeatSaberUI::CreateVerticalLayoutGroup(get_rectTransform());
    layout->set_childAlignment(UnityEngine::TextAnchor::UpperCenter);
    layout->set_childControlHeight(true);
    layout->set_childForceExpandHeight(false);

    BeatSaberUI::CreateToggle(layout->get_rectTransform(), "Hide Miss Text", getConfig().config["hideText"].GetBool(),
        il2cpp_utils::MakeDelegate<UnityAction_1<bool>*>(classof(UnityAction_1<bool>*), this, onChangeMissText));

    BeatSaberUI::CreateToggle(layout->get_rectTransform(), "Hide Combo Break", getConfig().config["hideBreakLines"].GetBool(),
        il2cpp_utils::MakeDelegate<UnityAction_1<bool>*>(classof(UnityAction_1<bool>*), this, onChangeComboBreak));

    BeatSaberUI::CreateToggle(layout->get_rectTransform(), "Subtle No Fail", getConfig().config["subtleNoFail"].GetBool(),
        il2cpp_utils::MakeDelegate<UnityAction_1<bool>*>(classof(UnityAction_1<bool>*), this, onChangeSubtleNoFail));
}

void SettingsViewController::DidDeactivate(bool removedFromHierarchy, bool systemScreenDisabling) {
    getConfig().Write();
    updateVals();
}