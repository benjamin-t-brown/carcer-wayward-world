#pragma once

#include "../UiLayer.h"
#include "ui/minipages/MinipageSpellCast.h"
#include <string_view>

namespace db {
class Database;
}

namespace layers {

/** Combat spell-cast list for the active party member's known spells. */
class LayerSpellCast : public UiLayer {
  bmin::String chId;
  static ui::MinipageSpellCastSpell makeSpellEntry(const db::Database& database,
                                                   const bmin::String& spellName);

public:
  constexpr static std::string_view LAYER_ID = "layer_spell_cast";

  explicit LayerSpellCast(sdl2w::Window* _window, const bmin::String& chId);
  ~LayerSpellCast() override = default;

  void onKeyDown(std::string_view key, int keyCode) override;
};

} // namespace layers
