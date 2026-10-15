#pragma once

#include "actions/combat/ModifyAP.hpp"
#include "actions/general/PlaySound.hpp"
#include "actions/world/ModifyPartyMemberHp.hpp"
#include "bmin/String.h"
#include "bmin/StringInterop.h"
#include "game/combat/AbilityOverrideMerge.h"
#include "game/combat/Damage.h"
#include "game/inventory/ItemUseRules.h"
#include "model/Combat.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/Player.h"
#include "model/templates/Abilities.hpp"
#include "model/templates/AbilityTypes.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.hpp"
#include "state/LayerRequest.h"
#include "state/State.hpp"

namespace state {

namespace actions {

class UiUseInventoryItem : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiUseInventoryItem; }
  bmin::String characterPlayerId;
  bmin::String itemId;

  void playDepictionSounds(const model::AbilityDepiction& depiction) {
    if (!depiction.startSound.empty()) {
      PlaySound(depiction.startSound).execute(state);
    }
    if (!depiction.dmgSound.empty()) {
      PlaySound(depiction.dmgSound).execute(state);
    }
  }

  void consumeOneUnlessIndestructable(model::CharacterPlayer& partyMember,
                                      const model::ItemTemplate& itemTemplate) {
    if (itemTemplate.indestructable) {
      return;
    }
    model::characterPlayerRemoveItemFromInventoryById(partyMember, itemId, 1);
  }

  void act() override {
    auto& localState = *state;
    auto* partyMember =
        model::playerFindPartyMemberById(localState.player, characterPlayerId);
    if (!partyMember) {
      LOG(WARN) << "UiUseInventoryItem::act: party member not found" << LOG_ENDL;
      return;
    }

    bmin::String itemTemplateName;
    int quantity = 0;
    for (const auto& item : partyMember->inventory) {
      if (item.id == itemId) {
        quantity = item.quantity;
        itemTemplateName = item.itemName;
        break;
      }
    }
    if (quantity < 1 || itemTemplateName.empty()) {
      LOG(WARN) << "UiUseInventoryItem::act: item not found in inventory" << LOG_ENDL;
      return;
    }

    auto* database = getDatabase();
    if (!database) {
      LOG(WARN) << "UiUseInventoryItem::act: database is nullptr" << LOG_ENDL;
      return;
    }

    const auto* itemTemplate =
        database->findItemTemplate(bmin::toStringView(itemTemplateName));
    if (!itemTemplate) {
      LOG(WARN) << "UiUseInventoryItem::act: missing item template "
                << itemTemplateName << LOG_ENDL;
      return;
    }

    const auto combatActive = localState.world.combat.active;
    const auto mapType = game::resolveActiveMapType(
        localState.world.activeMap, localState.mapInstances, database, localState.player);
    if (!game::isItemUsableInContext(itemTemplate->itemUsability, combatActive,
                                     mapType)) {
      return;
    }

    if (itemTemplate->useSpecialEvent.has_value() &&
        !itemTemplate->useSpecialEvent->empty()) {
      localState.triggers.pendingSpecialEventId = *itemTemplate->useSpecialEvent;
      consumeOneUnlessIndestructable(*partyMember, *itemTemplate);
      removeLayerRequest(localState, LayerId::InventoryContext);
      return;
    }

    if (!itemTemplate->useAbility.has_value() ||
        itemTemplate->useAbility->abilityName.empty()) {
      return;
    }

    const auto* baseAbility = database->findAbilityTemplate(
        bmin::toStringView(itemTemplate->useAbility->abilityName));
    if (!baseAbility) {
      LOG(WARN) << "UiUseInventoryItem::act: missing ability "
                << itemTemplate->useAbility->abilityName << LOG_ENDL;
      return;
    }

    if (baseAbility->targetSelect.targetType !=
        model::TargetSelectType::TARGET_SELF) {
      LOG(WARN) << "UiUseInventoryItem::act: only TARGET_SELF supported for v1, got "
                << model::targetSelectTypeToString(baseAbility->targetSelect.targetType)
                << LOG_ENDL;
      return;
    }

    auto ability = model::AbilityTemplate{*baseAbility};
    const auto& useAbility = *itemTemplate->useAbility;
    if (!useAbility.restoreOverrides.empty()) {
      game::mergeAbilityRestoreOverrides(ability, useAbility.restoreOverrides);
    }
    if (!useAbility.dmgOverrides.empty()) {
      game::mergeAbilityAttackDmgOverrides(ability, useAbility.dmgOverrides);
    }

    auto hpDelta = int{0};
    auto mpDelta = int{0};
    for (const auto& restore : ability.restores) {
      const auto amount =
          game::calculateAbilityRestoreAmount(restore, partyMember->stats);
      switch (restore.restoreWhich) {
      case model::CurrentStatEnum::CURRENT_STAT_HP:
        hpDelta += amount;
        break;
      case model::CurrentStatEnum::CURRENT_STAT_MANA:
        mpDelta += amount;
        break;
      case model::CurrentStatEnum::CURRENT_STAT_AP:
      case model::CurrentStatEnum::CURRENT_STAT_AC:
        break;
      }
    }

    const auto maxHp = partyMember->params.combat.hp;
    hpDelta = model::appliedHpDelta(partyMember->currentHp, maxHp, hpDelta);
    if (hpDelta != 0) {
      ModifyPartyMemberHp(characterPlayerId, hpDelta).execute(state);
    }

    if (mpDelta != 0) {
      const auto maxMp = partyMember->params.combat.mp;
      auto nextMp = partyMember->currentMp + mpDelta;
      if (maxMp > 0 && nextMp > maxMp) {
        nextMp = maxMp;
      }
      if (nextMp < 0) {
        nextMp = 0;
      }
      partyMember->currentMp = nextMp;
    }

    playDepictionSounds(ability.depiction);

    if (combatActive && ability.apCost != 0) {
      ModifyAP(characterPlayerId, -ability.apCost).execute(state);
    }

    consumeOneUnlessIndestructable(*partyMember, *itemTemplate);
    removeLayerRequest(localState, LayerId::InventoryContext);
  }

public:
  UiUseInventoryItem(bmin::String _characterPlayerId, bmin::String _itemId)
      : characterPlayerId(std::move(_characterPlayerId)), itemId(std::move(_itemId)) {}
};

} // namespace actions

} // namespace state
