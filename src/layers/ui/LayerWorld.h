#pragma once

#include "../UiLayer.h"
#include "WorldInputController.h"
#include "WorldViewSync.h"
#include "bmin/String.h"
#include <string_view>

namespace layers {

class LayerWorld : public UiLayer {
private:
  float mapScale = 1.f;

  WorldViewSync viewSync{*this};
  WorldInputController inputController{*this};

  void alignMapView();

public:
  constexpr static std::string_view LAYER_ID = "layer_world";

  explicit LayerWorld(sdl2w::Window* _window);
  virtual ~LayerWorld() = default;

  void onKeyDown(std::string_view key, int keyCode) override;
  void onKeyUp(std::string_view key, int keyCode) override;
  void onMouseDown(int x, int y, int button) override;
  void onMouseHover(int x, int y) override;
  void syncFromState();
  void setMapScale(float scale);
  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers
