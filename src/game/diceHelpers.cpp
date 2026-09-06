module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <cstdlib>

module carcer.game.combat;
import bmin.containers;
import carcer.data;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

namespace game {

int getNumDiceSides(model::Dice dice) {
  switch (dice) {
  case model::Dice::D0:
    return 0;
  case model::Dice::D2:
    return 2;
  case model::Dice::D4:
    return 4;
  case model::Dice::D6:
    return 6;
  case model::Dice::D8:
    return 8;
  case model::Dice::D10:
    return 10;
  case model::Dice::D12:
    return 12;
  case model::Dice::D20:
    return 20;
  case model::Dice::D100:
    return 100;
  }
  return 0;
}

int rollDice(model::Dice dice) {
  const auto sides = getNumDiceSides(dice);
  if (sides <= 0) {
    return 0;
  }
  return 1 + (std::rand() % sides);
}

int rollDiceList(const bmin::DynArray<model::Dice>& diceList) {
  auto total = int{0};
  for (const auto& dice : diceList) {
    total += rollDice(dice);
  }
  return total;
}

} // namespace game
