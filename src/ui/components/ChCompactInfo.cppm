module;
#include <cstddef>
#include <cstdint>
#include <utility>
#if __has_include(<SDL.h>)
#include <SDL.h>
#include <SDL_pixels.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#endif

export module carcer.ui.components:ChCompactInfo;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.core;
import sdl2w;
#include "macros.h"

export {

// --- from ui/components/ChCompactInfo.h ---
// IWYU pragma: keep

namespace ui {

struct ChCompactInfoProps {
  bmin::String characterSpriteName;
  bmin::DynArray<bmin::String> statusEffectSpriteNames;

  SDL_Color spriteBgColor = Colors::OffWhite;
  SDL_Color spriteBorderColor1 = Colors::LightGrey;
  SDL_Color spriteBorderColor2 = Colors::White;
  int spriteBorderSize = 1;
  int spriteBoxSize = 36;

  int statusIconSize = 12;
  int numStatusColumns = 2;

  int hp = 0;
  int mana = 0;
  bool isSelected = false;
  int padding = 4;
  sdl2w::TextSize fontSize = sdl2w::TEXT_SIZE_18;
};

// ChCompactInfo component - renders a character sprite, status effect icons,
// and health/mana values in a bordered box.
class ChCompactInfo : public UiElement {
private:
  ChCompactInfoProps props;

  int fontHeight = 20;

  int getContentWidth() const;
  int getContentHeight() const;

public:
  ChCompactInfo(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ChCompactInfo() override = default;

  void setProps(const ChCompactInfoProps& _props);
  ChCompactInfoProps& getProps();
  const ChCompactInfoProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export
