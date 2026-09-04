module;
#include <string_view>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <string>
#include <typeinfo>
#include <typeindex>

export module carcer.layers:LayerMagic;
export import carcer.layers.Layer;
export import carcer.db;
export import carcer.model.templates;
export import carcer.ui.pages.PageMagicSetup;
import sdl2w;
import carcer.actions;
import carcer.game.combat;
import carcer.model.instances;
import carcer.ui.helpers;
import bmin.containers;
import bmin.string_interop;
#include "macros.h"

export {

namespace layers {

class LayerMagic : public Layer {
private:
  static ui::PageMagicSetupSpellEntry makeSpellEntry(const db::Database& database,
                                                     const bmin::String& spellName);
  static ui::PageMagicSetupRuneSlot makeRuneSlotFromRuneType(model::RuneType runeType);

public:
  explicit LayerMagic(sdl2w::Window* _window);
  virtual ~LayerMagic() = default;

  void onKeyDown(std::string_view key, int keyCode) override;

  void syncMagicPartyMember();
};

} // namespace layers

} // export

namespace layers {

ui::PageMagicSetupSpellEntry LayerMagic::makeSpellEntry(const db::Database& database,
                                                        const bmin::String& spellName) {
  ui::PageMagicSetupSpellEntry entry;
  entry.id = spellName;

  const auto* spell = database.findSpellTemplate(bmin::toStringView(spellName));
  if (spell == nullptr) {
    LOG(ERROR) << "LayerMagic::makeSpellEntry: spell template not found: " << spellName
               << LOG_ENDL;
    entry.label = spellName;
    return entry;
  }

  const auto* ability =
      database.findAbilityTemplate(bmin::toStringView(spell->abilityName));
  entry.label = spell->label;
  if (entry.label.empty() && ability != nullptr) {
    entry.label = ability->label;
  }
  entry.iconSprite = spell->icon;
  if (entry.iconSprite.empty() && ability != nullptr) {
    entry.iconSprite = ability->icon;
  }
  // entry.manaCost = model::spellAbilityManaCost(*spell, database);
  entry.manaCost = ability->costValue;
  for (const auto& req : spell->requiredRunes) {
    const auto sprite = model::runeTypeToSpriteName(req.type);
    const int count = req.count > 0 ? req.count : 1;
    for (int i = 0; i < count; ++i) {
      entry.requiredRuneSprites.push_back(sprite);
    }
  }
  return entry;
}

ui::PageMagicSetupRuneSlot LayerMagic::makeRuneSlotFromRuneType(
    model::RuneType runeType) {
  ui::PageMagicSetupRuneSlot slot{.filled = true};
  slot.iconSprite = model::runeTypeToSpriteName(runeType);
  return slot;
}

LayerMagic::LayerMagic(sdl2w::Window* _window) : Layer(_window, state::LayerId::Magic) {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto pageMagicSetup = new ui::PageMagicSetup(window);
  pageMagicSetup->setId("pageMagicSetup");

  auto [windowWidth, windowHeight] = window->getDims();
  const auto scale = 1.f;

  pageMagicSetup->setPos(0, 0);
  pageMagicSetup->setScale(scale);
  auto pageInitProps = pageMagicSetup->getProps();
  // Window dims; PageMagicSetup → ModalStandard default CappedCentered fits/centers.
  pageInitProps.width = static_cast<int>(windowWidth / scale);
  pageInitProps.height = static_cast<int>(windowHeight / scale);
  pageMagicSetup->setProps(pageInitProps);

  addUiElement(pageMagicSetup);

  syncMagicPartyMember();

  subscribeAction<state::actions::UiSetCurrentPartyMemberMagic>(
      [this](auto&, auto&) { syncMagicPartyMember(); });
  subscribeAction<state::actions::UiSetSpellReady>(
      [this](auto&, auto&) { syncMagicPartyMember(); });
  subscribeAction<state::actions::UiCommitEquipRunes>(
      [this](auto&, auto&) { syncMagicPartyMember(); });
  subscribeAction<state::actions::UiCancelEquipRunes>(
      [this](auto&, auto&) { syncMagicPartyMember(); });
}

void LayerMagic::onKeyDown(std::string_view key, int /*keyCode*/) {
  if (getState() != LayerState::ON) {
    return;
  }
  auto stateManager = getStateManager();
  if (!stateManager) {
    remove();
    return;
  }

  if (const auto partyIndex = ui::getPartyMemberIndexFromKey(key)) {
    if (*partyIndex < static_cast<int>(stateManager->getState().player.party.size())) {
      stateManager->enqueueAction(
          stateManager->getActionData(),
          new state::actions::UiSetCurrentPartyMemberMagic(*partyIndex),
          0);
    }
    return;
  }

  if (!ui::isCancelActionKey(key)) {
    return;
  }
  stateManager->enqueueAction(
      stateManager->getActionData(),
      new state::actions::UiRemoveLayer(state::LayerId::Magic),
      0);
}

void LayerMagic::syncMagicPartyMember() {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto stateManager = getStateManager();
  auto* database = getDatabase();
  if (!database) {
    LOG(ERROR) << "LayerMagic::syncMagicPartyMember: database is nullptr" << LOG_ENDL;
    remove();
    return;
  }

  auto& player = stateManager->getState().player;
  auto magicPartyMember =
      model::playerFindPartyMemberByIndex(player, player.currentPartyMemberMagicIndex);

  if (!magicPartyMember) {
    LOG(ERROR) << "LayerMagic::syncMagicPartyMember: party member is nullptr" << LOG_ENDL;
    remove();
    return;
  }

  auto pageMagicSetup = getUiElement<ui::PageMagicSetup>("pageMagicSetup");
  if (!pageMagicSetup) {
    LOG(ERROR) << "LayerMagic::syncMagicPartyMember: pageMagicSetup is nullptr"
               << LOG_ENDL;
    return;
  }

  auto pageProps = pageMagicSetup->getProps();
  pageProps.characterPlayerId = magicPartyMember->instanceId;
  pageProps.characterPlayerLabel = magicPartyMember->params.label;
  pageProps.partyMemberMagicIndex = player.currentPartyMemberMagicIndex;
  pageProps.partyMembers.clear();
  for (const auto& member : player.party) {
    pageProps.partyMembers.pushBack(
        {.spriteName = model::characterPlayerGetSprite(member)});
  }
  pageProps.characterPlayerSprite = model::characterPlayerGetSprite(*magicPartyMember);

  pageProps.selectedRuneSlotIndex = -1;

  pageProps.elementCounts.clear();
  for (int i = 0; i < model::kRuneTypeCount; ++i) {
    const auto runeType = model::runeTypeFromIndex(i);
    const int available =
        model::characterPlayerCountAvailableRunesOfType(*magicPartyMember, runeType);
    const int equipped =
        model::characterPlayerCountEquippedRunesOfType(*magicPartyMember, runeType);
    pageProps.elementCounts.pushBack(ui::PageMagicSetupElementCount{
        .iconSprite = model::runeTypeToSpriteName(runeType),
        // Remaining unequipped (same as Equip Runes modal center counts).
        .count = available > equipped ? available - equipped : 0,
    });
  }

  // Single spell list; ready = equipped runes meet requiredRunes (shown as "(r)").
  pageProps.spells.clear();
  // TODO fix spell list
  for (const auto& spellName : magicPartyMember->knownSpells) {
    auto entry = makeSpellEntry(*database, spellName);
    entry.ready = true;
    pageProps.spells.pushBack(entry);
  }

  pageProps.runeSlots.clear();
  for (size_t i = 0; i < model::CharacterPlayer::kRuneSlotCount; ++i) {
    if (i < magicPartyMember->equippedRunes.size()) {
      pageProps.runeSlots.pushBack(
          makeRuneSlotFromRuneType(magicPartyMember->equippedRunes[i]));
    } else {
      pageProps.runeSlots.pushBack(ui::PageMagicSetupRuneSlot{.filled = false});
    }
  }

  pageMagicSetup->setProps(pageProps);
}

} // namespace layers
