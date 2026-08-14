#pragma once

#include "bmin/DynArray.h"
#include "model/templates/AbilityTypes.h"

namespace game {

int getNumDiceSides(model::Dice dice);
int rollDice(model::Dice dice);
int rollDiceList(const bmin::DynArray<model::Dice>& diceList);

} // namespace game
