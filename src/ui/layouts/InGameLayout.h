#pragma once

#include "../UiElement.h"
#include "model/WorldActions.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include <vector>

namespace ui {

enum class InGameBorderType { Wide, Narrow };

// InGameLayout layout properties
struct InGameLayoutProps {
  std::vector<model::WorldActionType> worldActionTypes;
  float actionButtonScale = 1.f;
  InGameBorderType borderType = InGameBorderType::Wide;
};

// InGameLayout layout - renders an in-game layout with background, border, title,
// subtitle, and children Uses Position, Size, Scale from BaseStyle
class InGameLayout : public UiElement {
private:
  InGameLayoutProps props;

public:
  InGameLayout(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~InGameLayout() override = default;

  void setProps(const InGameLayoutProps& _props);
  InGameLayoutProps& getProps();
  const InGameLayoutProps& getProps() const;

  void setTitleElement(UiElement* _titleElement);
  UiElement* getTitleElement();

  void build() override;
  void render(int dt) override;
};

} // namespace ui
