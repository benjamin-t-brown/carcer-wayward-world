#include "LayerCharacterExamine.h"
#include "bmin/StringInterop.h"
#include "bmin/UniquePtr.h"
#include "db/Database.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/instances/Player.h"
#include "sdl2w/Logger.h"
#include "actions/navigation/UiRemoveLayer.hpp"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/helpers/keyboardShortcuts.h"
#include "ui/helpers/uiSounds.h"
#include "ui/pages/PageCharacter.h"

namespace layers {

namespace {

void fillDisplayCharacter(model::CharacterPlayer& display,
                          const model::CharacterInstance& instance,
                          model::Player& player,
                          const db::Database& database) {
  if (auto* partyMember = model::playerFindPartyMemberById(player, instance.id)) {
    display = *partyMember;
    return;
  }

  display.instanceId = instance.id;
  display.templateName = instance.templateName;
  display.name = instance.name;
  if (display.name.empty()) {
    display.name = instance.label;
  }
  display.stats = instance.stats;
  display.currentHp = instance.currentHp;
  display.currentMp = instance.currentMp;
  try {
    display.params =
        database.getCharacterTemplate(bmin::toStringView(instance.templateName));
    if (display.name.empty()) {
      display.name =
          display.params.label.empty() ? display.params.name : display.params.label;
    }
  } catch (...) {
  }
}

ui::PageCharacterStatusEntry makeStatusEntry(const model::AppliedStatusEffect& applied,
                                             const db::Database* database) {
  ui::PageCharacterStatusEntry entry;
  entry.name = applied.statusEffectName;
  entry.remainingTurns = applied.remainingTurns;
  if (database == nullptr) {
    return entry;
  }
  const auto* statusTemplate = database->findStatusEffectTemplate(
      bmin::toStringView(applied.statusEffectName));
  if (statusTemplate == nullptr) {
    return entry;
  }
  if (!statusTemplate->name.empty()) {
    entry.name = statusTemplate->name;
  }
  entry.description = statusTemplate->description;
  entry.iconSprite = statusTemplate->icon;
  return entry;
}

} // namespace

LayerCharacterExamine::LayerCharacterExamine(sdl2w::Window* _window,
                                             const bmin::String& characterId)
    : UiLayer(_window, LAYER_ID) {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  if (characterId.empty()) {
    LOG(ERROR) << "LayerCharacterExamine: characterId is empty" << LOG_ENDL;
    remove();
    return;
  }

  auto* stateManager = getStateManager();
  auto* database = getDatabase();
  if (!stateManager || !database) {
    remove();
    return;
  }

  auto& state = stateManager->getState();
  const model::CharacterInstance* mapCharacter = nullptr;
  for (size_t i = 0; i < state.world.activeMap.characters.size(); i++) {
    if (state.world.activeMap.characters[i].id == characterId) {
      mapCharacter = &state.world.activeMap.characters[i];
      break;
    }
  }
  if (mapCharacter == nullptr) {
    LOG(ERROR) << "LayerCharacterExamine: character not found: " << characterId
               << LOG_ENDL;
    remove();
    return;
  }

  fillDisplayCharacter(displayCharacter, *mapCharacter, state.player, *database);

  auto pageCharacter = new ui::PageCharacter(window);
  pageCharacter->setId("pageCharacter");

  auto [windowWidth, windowHeight] = window->getDims();
  const auto scale = 1.f;
  pageCharacter->setPos(0, 0);
  pageCharacter->setScale(scale);

  ui::PageCharacterProps pageProps;
  pageProps.width = static_cast<int>(windowWidth / scale);
  pageProps.height = static_cast<int>(windowHeight / scale);
  pageProps.characterPlayer = &displayCharacter;
  pageProps.allowStatModification = false;
  for (size_t i = 0; i < mapCharacter->statusEffects.size(); i++) {
    pageProps.statusEffects.pushBack(
        makeStatusEntry(mapCharacter->statusEffects[i], database));
  }
  pageCharacter->setProps(pageProps);

  addUiElement(bmin::UniquePtr<ui::UiElement>(pageCharacter));

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(bmin::UniquePtr<ui::UiElement>(floatingNotificationSection));
}

void LayerCharacterExamine::onKeyDown(std::string_view key, int /*keyCode*/) {
  if (getState() != LayerState::ON) {
    return;
  }
  auto stateManager = getStateManager();
  if (!stateManager) {
    remove();
    return;
  }
  if (!ui::isCancelActionKey(key)) {
    return;
  }
  ui::playButtonSound(window);
  stateManager->enqueueAction(
      state::makeAction<state::actions::UiRemoveLayer>(
          bmin::String(LAYER_ID.data(), LAYER_ID.size())),
      0);
}

} // namespace layers
