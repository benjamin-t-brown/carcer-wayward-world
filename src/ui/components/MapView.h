#pragma once

#include "../UiElement.h"
#include "model/instances/World.hpp"
#include "state/DatabaseInterface.h"
#include "state/State.hpp"
#include <functional>
#include <optional>

namespace game {
class ActiveMapOrchestrator;
}

namespace sdl2w {
class Sprite;
class Store;
} // namespace sdl2w

namespace ui {

struct MapViewProps {
  int width = 0;
  int height = 0;
};

// Draws State.world.activeMap tiles (from stitched MapInstances), items,
// characters, and damage particles into a clipped content rect using
// State.world.camera.camX / camY (map pixel space). Does not own or mutate camera.
class MapView : public UiElement, public state::DatabaseInterface {
private:
  MapViewProps props;

  SDL_Color mapFogColor{0, 0, 0, 128};
  SDL_Color mapUnexploredColor{0, 0, 0, 255};
  SDL_Color actionAimFillColor{66, 202, 253, 64};
  SDL_Color actionAimOutlineColor{66, 202, 253, 220};
  SDL_Color combatTurnOutlineColor{160, 160, 160, 220};

  // Fog frill sprite indices into frills.png (frills_0 unused).
  static constexpr int frillTop = 1;
  static constexpr int frillBottom = 2;
  static constexpr int frillRight = 3;
  static constexpr int frillLeft = 4;
  static constexpr int frillInnerTopRight = 5;
  static constexpr int frillInnerTopLeft = 6;
  static constexpr int frillInnerBottomLeft = 7;
  static constexpr int frillInnerBottomRight = 8;
  static constexpr int frillOuterTopLeft = 9;
  static constexpr int frillOuterTopRight = 10;
  static constexpr int frillOuterBottomRight = 11;
  static constexpr int frillOuterBottomLeft = 12;

  bmin::Map<bmin::String, bmin::UniquePtr<sdl2w::Animation>> animations;

  // Out-of-map / missing / empty / unexplored → solid black.
  static bool isUnexploredBlack(game::ActiveMapOrchestrator& orch,
                                int worldX,
                                int worldY,
                                int mapLayer);

  // True when a neighbor should receive a vision-edge frill from a visible tile:
  // unexplored black OR explored-but-not-visible (semi fog).
  static bool isNotCurrentlyVisible(game::ActiveMapOrchestrator& orch,
                                    int worldX,
                                    int worldY,
                                    int mapLayer);

  // edgeAgainstHidden: visible tiles use not-currently-visible neighbors;
  // fogged tiles use unexplored-black neighbors only (soft edge into the void).
  static void drawFogFrills(game::ActiveMapOrchestrator& orch,
                            sdl2w::Store& store,
                            int worldX,
                            int worldY,
                            int mapLayer,
                            int screenX,
                            int screenY,
                            bool edgeAgainstHidden,
                            const std::function<void(sdl2w::Sprite&, int, int)>& drawSprite);

  void renderDamageParticles(state::State& state,
                             sdl2w::Draw& draw,
                             sdl2w::Store& store,
                             int contentX,
                             int contentY,
                             int spriteW,
                             int spriteH,
                             int fontScale);

  void renderProjectiles(const model::World& world,
                         sdl2w::Draw& draw,
                         sdl2w::Store& store,
                         int contentX,
                         int contentY,
                         int spriteW,
                         int spriteH);

  void addAnimation(const bmin::String& animationName);
  sdl2w::Animation* getAnimation(const bmin::String& animationName);
  sdl2w::Animation* upsertAnimation(const bmin::String& animationName);

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
