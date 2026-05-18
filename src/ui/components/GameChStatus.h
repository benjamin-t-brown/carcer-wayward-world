#pragma once

#include "../UiElement.h"
#include <SDL2/SDL_pixels.h>
#include <string>
#include <vector>

namespace ui {

struct GameChStatusProps {
  std::string characterSpriteName;
  std::vector<std::string> statusEffectSpriteNames;
  int health = 0;
  int mana = 0;

  int padding = 4;
  int rowGap = 4;
  int spriteSize = 32;
  int statusIconSize = 16;
  int statusIconGap = 2;
  int statusGridCols = 4;

  SDL_Color backgroundColor = Colors::Black;
  SDL_Color borderColor = Colors::LightGrey;
  int borderSize = 1;
};

// GameChStatus component - renders a character sprite, status effect icons,
// and health/mana values in a bordered box.
class GameChStatus : public UiElement {
private:
  GameChStatusProps props;

public:
  GameChStatus(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~GameChStatus() override = default;

  void setProps(const GameChStatusProps& _props);
  GameChStatusProps& getProps();
  const GameChStatusProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
