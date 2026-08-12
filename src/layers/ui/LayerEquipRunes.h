#pragma once

#include "../Layer.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "model/templates/RuneTypes.h"

namespace layers {

class LayerEquipRunes : public Layer {
private:
  bmin::String characterPlayerId;
  bmin::DynArray<model::RuneType> equippedSnapshot;

  void syncFromCharacter();

public:
  constexpr static std::string_view LAYER_ID = "layer_equip_runes";

  explicit LayerEquipRunes(sdl2w::Window* _window,
                           const bmin::String& characterPlayerId);
  ~LayerEquipRunes() override = default;

  void restoreSnapshot();
  void onKeyDown(std::string_view key, int keyCode) override;
};

} // namespace layers
