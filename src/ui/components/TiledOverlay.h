#pragma once

#include "ui/UiElement.h"
#include "bmin/String.h"

#if defined(MIYOOA30) || defined(MIYOOMINI)
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

namespace ui {

struct TiledOverlayProps {
  int width = 0;
  int height = 0;
  bmin::String spriteName = "ui_overlay_256";
  int alpha = 40;
};

// Tiles a sprite into a Quad-sized offscreen texture (edges clip to the texture),
// then blits it with alpha. Decorative — does not take input.
class TiledOverlay : public UiElement {
private:
  TiledOverlayProps props;
  SDL_Texture* renderTexture = nullptr;
  int currentWidth = 0;
  int currentHeight = 0;

  void createRenderTexture();
  void destroyRenderTexture();

public:
  TiledOverlay(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~TiledOverlay() override;

  void setProps(const TiledOverlayProps& _props);
  TiledOverlayProps& getProps();
  const TiledOverlayProps& getProps() const;

  bool checkMouseDownEvent(int mouseX,
                           int mouseY,
                           int button,
                           bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkMouseUpEvent(int mouseX,
                         int mouseY,
                         int button,
                         bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkHoverEvent(int mouseX,
                       int mouseY,
                       bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkMouseWheelEvent(int mouseX,
                            int mouseY,
                            int delta,
                            bmin::DynArray<UiElement*> additionalElements = {}) override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
