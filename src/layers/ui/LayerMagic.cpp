#include "LayerMagic.h"
#include "bmin/String.h"
#include "bmin/StringInterop.h"
#include "game/combat/SpellRules.h"
#include "model/instances/CharacterPlayer.h"
#include "model/templates/RuneTypes.h"
#include "sdl2w/Logger.h"
#include "actions/navigation/UiCancelEquipRunes.hpp"
#include "actions/navigation/UiCommitEquipRunes.hpp"
#include "actions/navigation/UiRemoveLayer.hpp"
#include "actions/navigation/UiSetCurrentPartyMemberMagic.hpp"
#include "actions/navigation/UiSetSpellReady.hpp"
#include "ui/helpers/keyboardShortcuts.h"
#include "ui/pages/PageMagicSetup.h"

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
      entry.requiredRuneSprites.pushBack(sprite);
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

LayerMagic::LayerMagic(sdl2w::Window* _window) : UiLayer(_window, LAYER_ID) {
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

  subscribeAction<state::ActionEvent::UiSetCurrentPartyMemberMagic>(
      [this](auto&, auto&) { syncMagicPartyMember(); });
  subscribeAction<state::ActionEvent::UiSetSpellReady>(
      [this](auto&, auto&) { syncMagicPartyMember(); });
  subscribeAction<state::ActionEvent::UiCommitEquipRunes>(
      [this](auto&, auto&) { syncMagicPartyMember(); });
  subscribeAction<state::ActionEvent::UiCancelEquipRunes>(
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
      stateManager->enqueueAction(state::makeAction<state::actions::UiSetCurrentPartyMemberMagic>(*partyIndex),
          0);
    }
    return;
  }

  if (!ui::isCancelActionKey(key)) {
    return;
  }
  stateManager->enqueueAction(state::makeAction<state::actions::UiRemoveLayer>(bmin::String(LAYER_ID.data(), LAYER_ID.size())),
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
