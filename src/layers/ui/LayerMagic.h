#pragma once

#include "../UiLayer.h"
#include "bmin/String.h"
#include "db/Database.h"
#include "model/instances/CharacterPlayer.h"
#include "ui/pages/PageMagicSetup.h"

namespace layers {

class LayerMagic : public UiLayer {
private:
  static ui::PageMagicSetupSpellEntry makeSpellEntry(const db::Database& database,
                                                     const bmin::String& spellName);
  static ui::PageMagicSetupRuneSlot makeRuneSlotFromRuneType(model::RuneType runeType);

public:
  constexpr static std::string_view LAYER_ID = "layer_magic";

  explicit LayerMagic(sdl2w::Window* _window);
  virtual ~LayerMagic() = default;

  void onKeyDown(std::string_view key, int keyCode) override;

  void syncMagicPartyMember();
};

} // namespace layers
