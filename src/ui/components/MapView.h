#pragma once

#include "../UiElement.h"
#include "state/DatabaseInterface.h"

namespace ui {

struct MapViewProps {
  int width = 0;
  int height = 0;
};

// Draws State.world.currentMap tiles, items, and characters into a clipped
// content rect using State.world.camX / camY (map pixel space). Does not own or
// mutate camera.
class MapView : public UiElement, public state::DatabaseInterface {
private:
  MapViewProps props;

public:
  MapView(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~MapView() override = default;

  void setProps(const MapViewProps& _props);
  MapViewProps& getProps();
  const MapViewProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
