#pragma once

#include "../UiLayer.h"
#include "bmin/String.h"
#include "model/instances/CharacterPlayer.h"

namespace layers {

class LayerCharacterExamine : public UiLayer {
  model::CharacterPlayer displayCharacter;

public:
  constexpr static std::string_view LAYER_ID = "layer_character_examine";

  explicit LayerCharacterExamine(sdl2w::Window* _window,
                                 const bmin::String& characterId);
  ~LayerCharacterExamine() override = default;

  void onKeyDown(std::string_view key, int keyCode) override;
};

} // namespace layers
