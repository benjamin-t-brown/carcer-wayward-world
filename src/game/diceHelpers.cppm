module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.game.diceHelpers;
export import bmin.containers;
import bmin.string_interop;
export import carcer.model.templates;

export {

// --- from game/diceHelpers.h ---
namespace game {

int getNumDiceSides(model::Dice dice);
int rollDice(model::Dice dice);
int rollDiceList(const bmin::DynArray<model::Dice>& diceList);

} // namespace game

} // export
