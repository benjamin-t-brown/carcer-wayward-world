#pragma once

#include "../UiLayer.h"
#include "bmin/String.h"
#include <string_view>

namespace layers {

class LayerSpellAllyTarget : public UiLayer {
  bmin::String casterId;
  bmin::String spellId;

public:
  constexpr static std::string_view LAYER_ID = "layer_spell_ally_target";

  LayerSpellAllyTarget(sdl2w::Window* _window,
                       const bmin::String& casterId,
                       const bmin::String& spellId);
  ~LayerSpellAllyTarget() override = default;

  void onKeyDown(std::string_view key, int keyCode) override;
};

} // namespace layers
