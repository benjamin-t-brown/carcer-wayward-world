#pragma once

#include "../UiElement.h"
#include "model/instances/World.h"
#include "state/DatabaseInterface.h"
#include <optional>

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

  SDL_Color mapFogColor{0, 0, 0, 128};
  SDL_Color mapUnexploredColor{0, 0, 0, 255};
  SDL_Color actionAimFillColor{66, 202, 253, 64};
  SDL_Color actionAimOutlineColor{66, 202, 253, 220};

public:
  MapView(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~MapView() override = default;

  void setProps(const MapViewProps& _props);
  MapViewProps& getProps();
  const MapViewProps& getProps() const;

  // Screen pixel → map tile using the inverse of MapView render math.
  // nullopt if outside content rect or outside map bounds.
  std::optional<model::TileXY> screenToTile(int screenX, int screenY) const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
