#pragma once

#include "../UiElement.h"
#include <SDL2/SDL_pixels.h>
#include <string>

#if defined(MIYOOA30) || defined(MIYOOMINI)
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

namespace ui {

// Quad-specific properties
struct QuadProps {
  SDL_Color bgColor = SDL_Color{0, 0, 0, 0};
  std::string bgSprite;
  SDL_Color borderColor = SDL_Color{0, 0, 0, 0};
  int borderSize = 0;
};

// Quad element - renders a stylized rectangle with children
// Uses Position, Size, Scale from BaseStyle
// Children are managed through base UiElement class
class Quad : public UiElement {
private:
  SDL_Texture* renderTexture = nullptr;
  int currentWidth = 0;
  int currentHeight = 0;

  QuadProps props;

  void createRenderTexture();
  void destroyRenderTexture();

public:
  Quad(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~Quad() override;

  // Setters and getters for quad-specific properties
  void setProps(const QuadProps& _props);
  QuadProps& getProps();
  const QuadProps& getProps() const;

  void updatePosition(int x, int y);

  bool checkMouseDownEvent(int mouseX, int mouseY, int button) override;
  bool checkMouseUpEvent(int mouseX, int mouseY, int button) override;
  bool checkHoverEvent(int mouseX, int mouseY) override;
  bool checkMouseWheelEvent(int mouseX, int mouseY, int delta) override;

  void build() override;
  void render() override;
};

} // namespace ui
