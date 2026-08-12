#pragma once

#include "bmin/String.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/Player.h"
#include "model/templates/RuneTypes.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

/** Live-adjust equipped runes while the Equip Runes modal is open (delta +1 / -1). */
class UiAdjustEquippedRune : public AbstractAction {
  bmin::String characterPlayerId;
  model::RuneType runeType = model::RuneType::HEAT;
  int delta = 0;

  void act() override {
    auto* characterPlayer =
        model::playerFindPartyMemberById(state->player, characterPlayerId);
    if (!characterPlayer) {
      LOG(WARN) << "UiAdjustEquippedRune::act: character not found "
                << characterPlayerId << LOG_ENDL;
      return;
    }

    if (delta > 0) {
      model::characterPlayerEquipRuneType(*characterPlayer, runeType);
    } else if (delta < 0) {
      model::characterPlayerUnequipOneRuneOfType(*characterPlayer, runeType);
    }
  }

public:
  UiAdjustEquippedRune(const bmin::String& _characterPlayerId,
                       model::RuneType _runeType,
                       int _delta)
      : characterPlayerId(_characterPlayerId), runeType(_runeType), delta(_delta) {}
};

} // namespace actions

} // namespace state
