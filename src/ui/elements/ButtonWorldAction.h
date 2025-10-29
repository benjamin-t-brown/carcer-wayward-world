#pragma once

#include "../UiElement.h"
#include "model/WorldActions.h"

namespace ui {

// ButtonWorldAction-specific properties
struct ButtonWorldActionProps {
  model::WorldActionType worldActionType;
};

struct ButtonWorldActionMapping {
  std::string label;
  int spriteIndex;
  bool isSmall = false;
};

// ButtonWorldAction element - renders a clickable button typically used to interact with
// the world Uses Position, Size, Scale from BaseStyle
class ButtonWorldAction : public UiElement {
private:
  ButtonWorldActionProps props;
  bool isInHoverMode = false;
  bool isInActiveMode = false;
  const std::string spriteSheetName = "ui_action_buttons";
  const int normalStartingSpriteIndex = 16;
  const int smallStartingSpriteIndex = 0;
  const int normalSpriteOffsetToActive = 16;
  const int smallSpriteOffsetToActive = 8;

  static ButtonWorldActionMapping
  getButtonWorldActionMapping(model::WorldActionType worldActionType);

public:
  bool isActive = false;
  ButtonWorldAction(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonWorldAction() override = default;

  static bool checkIfWorldActionButtonIsSmall(model::WorldActionType worldActionType);

  void setProps(const ButtonWorldActionProps& _props);
  ButtonWorldActionProps& getProps();
  const ButtonWorldActionProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
