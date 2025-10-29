#include "InGameLayout.h"
#include "lib/sdl2w/Draw.h"
#include "ui/components/BorderInGame.h"
#include "ui/elements/ButtonWorldAction.h"

namespace ui {

InGameLayout::InGameLayout(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Layout doesn't need special initialization
}

void InGameLayout::setProps(const InGameLayoutProps& _props) {
  props = _props;
  children.clear();
  build();
}

InGameLayoutProps& InGameLayout::getProps() { return props; }

const InGameLayoutProps& InGameLayout::getProps() const { return props; }

UiElement* InGameLayout::getChildById(const std::string& id) {
  for (auto& child : children) {
    if (child->getId() == id) {
      return child.get();
    }
  }
  return nullptr;
}

void InGameLayout::removeChildById(const std::string& id) {
  for (size_t i = 0; i < children.size(); i++) {
    auto& child = children[i];
    if (child->getId() == id) {
      children.erase(children.begin() + i);
      return;
    }
  }
}

const std::pair<int, int> InGameLayout::getDims() const {
  return {style.width, style.height};
}

void InGameLayout::build() {
  removeChildById("border");

  // Create border element
  auto border = std::make_unique<BorderInGame>(window, this);
  border->setId("border");
  BaseStyle borderStyle;
  borderStyle.x = style.x;
  borderStyle.y = style.y;
  borderStyle.width = style.width;
  borderStyle.height = style.height;
  border->setStyle(borderStyle);

  auto actionButtonsAreaLocation = border->getActionButtonsAreaLocation();
  auto buttonScale = 2.f;
  auto buttonWidthScaled = static_cast<int>(32. * buttonScale);
  auto buttonSmallIndex = 0;
  auto xAgg = 0;
  for (int i = 0; i < static_cast<int>(props.worldActionTypes.size()); i++) {
    auto worldActionType = props.worldActionTypes[i];
    auto button = std::make_unique<ButtonWorldAction>(window);
    auto isSmall = ButtonWorldAction::checkIfWorldActionButtonIsSmall(worldActionType);

    BaseStyle buttonStyle;
    buttonStyle.x = actionButtonsAreaLocation.first + xAgg;
    buttonStyle.y = actionButtonsAreaLocation.second;
    buttonStyle.scale = buttonScale;

    // keeps the xAgg at same position for two consecutive small buttons
    xAgg += buttonWidthScaled;
    if (isSmall) {
      if (buttonSmallIndex == 0) {
        xAgg -= buttonWidthScaled;
        buttonSmallIndex = 1;
      } else {
        buttonStyle.y += buttonWidthScaled / 2;
        buttonSmallIndex = 0;
      }
    }

    button->setStyle(buttonStyle);
    button->setId("worldActionButton_" + std::to_string(i));
    button->setProps(ButtonWorldActionProps{worldActionType});
    button->build();

    children.push_back(std::move(button));
  }

  // Insert border at the beginning
  children.insert(children.begin(), std::move(border));
  // TODO party area
}

void InGameLayout::setTitleElement(std::unique_ptr<UiElement> _titleElement) {
  removeChildById("title");
  auto borderElement = dynamic_cast<BorderInGame*>(getChildById("border"));
  // Add new title element
  if (_titleElement && borderElement) {
    BaseStyle& titleStyle = _titleElement->getStyle();
    auto titleLocation = borderElement->getTitleLocation();
    titleStyle.x = titleLocation.first;
    titleStyle.y = style.y + 44 / 2 - titleStyle.height / 2;
    _titleElement->setStyle(titleStyle);
    _titleElement->setId("title");
    children.push_back(std::move(_titleElement));
  }
}

UiElement* InGameLayout::getTitleElement() { return getChildById("title"); }

void InGameLayout::setSubtitleElement(std::unique_ptr<UiElement> _subtitleElement) {
  removeChildById("subtitle");
  auto borderElement = dynamic_cast<BorderInGame*>(getChildById("border"));
  // Add new subtitle element
  if (_subtitleElement && borderElement) {
    BaseStyle& subtitleStyle = _subtitleElement->getStyle();
    auto subtitleLocation = borderElement->getSubTitleLocation();
    subtitleStyle.x = subtitleLocation.first;
    subtitleStyle.y = subtitleLocation.second;
    _subtitleElement->setStyle(subtitleStyle);
    _subtitleElement->setId("subtitle");
    children.push_back(std::move(_subtitleElement));
  }
}

UiElement* InGameLayout::getSubtitleElement() { return getChildById("subtitle"); }

void InGameLayout::render(int dt) { UiElement::render(dt); }

} // namespace ui
