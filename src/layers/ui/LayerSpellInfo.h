#pragma once

#include "../Layer.h"
#include "bmin/String.h"

namespace layers {

class LayerSpellInfo : public Layer {
public:
  constexpr static std::string_view LAYER_ID = "layer_spell_info";

  explicit LayerSpellInfo(sdl2w::Window* _window, const bmin::String& spellName);
  ~LayerSpellInfo() override = default;

  void onKeyDown(std::string_view key, int keyCode) override;
};

} // namespace layers
