#pragma once

#include "../UiElement.h"
#include <string>
#include <utility>

namespace ui {

enum class TextBannerCorner { LEFT_TOP, LEFT_BOTTOM, RIGHT_TOP, RIGHT_BOTTOM };

struct TextBannerProps {
  std::pair<int, int> location = {0, 0};
  std::pair<int, int> dims = {0, 0};
  TextBannerCorner corner = TextBannerCorner::LEFT_TOP;
  std::string text;
  SDL_Color backgroundColor = Colors::Grey;
  int padding = 6;
  int outsetBorderSize = 0;
};

// TextBanner element - text with an outset rectangle background, placed in a corner
// of a container defined by location and dims props.
class TextBanner : public UiElement {
private:
  TextBannerProps props;

  std::pair<int, int> measureTextScaled() const;
  std::pair<int, int> calculateBannerScreenPosition(int bannerScaledWidth,
                                                      int bannerScaledHeight) const;

public:
  TextBanner(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~TextBanner() override = default;

  void setProps(const TextBannerProps& _props);
  TextBannerProps& getProps();
  const TextBannerProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
