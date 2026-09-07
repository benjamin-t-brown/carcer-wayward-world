module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <SDL.h>
#include <SDL_pixels.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <algorithm>

export module carcer.ui.screens.layouts;
export import bmin.containers;
export import carcer.ui.core;
export import carcer.ui.screens.runtime;
export import carcer.ui.widgets.composites;
export import carcer.state;
import bmin.string_interop;
import sdl2w;
import carcer.actions;
#include "macros.h"

export {

// --- from ui/layouts/ModalSmall.h ---
// IWYU pragma: keep

namespace ui {

// ModalSmall layout properties
struct ModalSmallProps {
  // For LayoutFit::CappedCentered (default), width/height are window dims.
  // For LayoutFit::FullBleed, width/height are the modal size as-is.
  int width = 0;
  int height = 0;
  LayoutFit layoutFit = LayoutFit::CappedCentered;
  SDL_Color backgroundColor = Colors::White;
  bmin::String iconSprite = "";
  /** Scales headerHeight (base 80), icon well (base 64), and the header icon sprite. */
  float iconScale = 1.f;
  bool enableCloseButton = true;
};

// ModalSmall layout - renders a small modal with background, border, title, subtitle, close
// button, and children Uses Position, Size, Scale from BaseStyle
class ModalSmall : public UiElement {
private:
  ModalSmallProps props;

  int getScaledButtonsAreaHeight();

public:
  static const int BUTTONS_AREA_HEIGHT = 50;

  ModalSmall(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ModalSmall() override = default;

  // Setters and getters for layout-specific properties
  void setProps(const ModalSmallProps& _props);
  ModalSmallProps& getProps();
  const ModalSmallProps& getProps() const;

  void setTitleElement(UiElement* _titleElement);
  UiElement* getTitleElement();
  UiElement* getCloseButtonElement();
  const std::pair<int, int> getContentDims();
  const std::pair<int, int> getContentLocation();
  const std::pair<int, int> getButtonsDims();
  const std::pair<int, int> getButtonsLocation();

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

ModalSmall::ModalSmall(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Layout doesn't need special initialization
}

void ModalSmall::setProps(const ModalSmallProps& _props) {
  props = _props;
  build();
}

ModalSmallProps& ModalSmall::getProps() { return props; }

const ModalSmallProps& ModalSmall::getProps() const { return props; }

int ModalSmall::getScaledButtonsAreaHeight() {
  auto borderElement = dynamic_cast<BorderModalSmall*>(getChildById("border"));
  if (borderElement == nullptr) {
    return 0;
  }
  return static_cast<int>(BUTTONS_AREA_HEIGHT * style.scale);
}

const std::pair<int, int> ModalSmall::getButtonsDims() {
  auto borderElement = dynamic_cast<BorderModalSmall*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  auto [contentW, _] = borderElement->getContentDims();
  return {contentW, getScaledButtonsAreaHeight()};
}

const std::pair<int, int> ModalSmall::getButtonsLocation() {
  auto borderElement = dynamic_cast<BorderModalSmall*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  auto [contentX, contentY] = borderElement->getContentLocation();
  auto [_, contentH] = borderElement->getContentDims();
  int buttonsH = getScaledButtonsAreaHeight();
  return {contentX, contentY + contentH - buttonsH};
}

const std::pair<int, int> ModalSmall::getContentDims() {
  auto borderElement = dynamic_cast<BorderModalSmall*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  auto [contentW, contentH] = borderElement->getContentDims();
  return {contentW, contentH - getScaledButtonsAreaHeight()};
}

const std::pair<int, int> ModalSmall::getContentLocation() {
  auto borderElement = dynamic_cast<BorderModalSmall*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  return borderElement->getContentLocation();
}

void ModalSmall::setTitleElement(UiElement* _titleElement) {
  removeChildById("title");
  auto borderElement = dynamic_cast<BorderModalSmall*>(getChildById("border"));
  // Add new title element
  if (_titleElement && borderElement) {
    auto [titleX, titleY] = borderElement->getTitleLocation();
    auto [titleWidth, titleHeight] = _titleElement->getDims();
    _titleElement->setPos(titleX, titleY - titleHeight / 2);
    _titleElement->setId("title");
    addChild(_titleElement);
  }
}

UiElement* ModalSmall::getTitleElement() { return getChildById("title"); }

UiElement* ModalSmall::getCloseButtonElement() { return getChildById("closeButton"); }

void ModalSmall::build() {
  removeChildById("border");
  removeChildById("closeButton");
  removeChildById("headerIcon");

  if (props.width > 0 && props.height > 0) {
    if (props.layoutFit == LayoutFit::CappedCentered) {
      const auto rect =
          computeCappedCenteredRect(props.width, props.height, ModalSizeClass::Small);
      style.x = rect.x;
      style.y = rect.y;
      style.width = rect.width;
      style.height = rect.height;
    } else {
      style.width = props.width;
      style.height = props.height;
    }
  }

  constexpr int baseHeaderHeight = 80;
  constexpr int baseIconSize = 64;
  const float iconScale = props.iconScale > 0.f ? props.iconScale : 1.f;
  const int headerHeight = std::max(1, static_cast<int>(baseHeaderHeight * iconScale));
  const int iconWellSize = std::max(1, static_cast<int>(baseIconSize * iconScale));

  int spriteW = 0;
  int spriteH = 0;
  if (!props.iconSprite.empty() && window) {
    const auto& sprite =
        window->getStore().getSprite(bmin::toStringView(props.iconSprite));
    spriteW = std::max(1, sprite.w);
    spriteH = std::max(1, sprite.h);
  }

  // Create border element
  auto border = new BorderModalSmall(window, this);
  border->setId("border");
  border->setPos(style.x, style.y);
  border->setScale(style.scale);
  border->setProps(BorderModalSmallProps{
      .width = style.width,
      .height = style.height,
      .headerHeight = headerHeight,
      .iconSize = iconWellSize,
  });
  addChild(border);

  if (props.enableCloseButton) {
    auto [closeX, closeY] = border->getCloseButtonLocation();

    auto modalClose = new ButtonClose(window, this);
    modalClose->setId("closeButton");
    modalClose->setPos(closeX, closeY);
    modalClose->setScale(style.scale);
    ui::ButtonCloseProps modalCloseProps;
    modalCloseProps.closeType = ui::CloseType::MODAL;
    modalClose->setProps(modalCloseProps);
    addChild(modalClose);
  }

  if (!props.iconSprite.empty() && spriteW > 0 && spriteH > 0) {
    const float drawScale = iconScale * style.scale;
    const int screenW = static_cast<int>(spriteW * drawScale);
    const int screenH = static_cast<int>(spriteH * drawScale);
    auto [centerX, centerY] = border->getIconLocationCenter();

    auto icon = new Quad(window, this);
    icon->setId("headerIcon");
    icon->setPos(centerX - screenW / 2, centerY - screenH / 2);
    icon->setScale(drawScale);
    icon->setProps(QuadProps{
        .width = spriteW,
        .height = spriteH,
        .bgSprite = props.iconSprite,
    });
    addChild(icon);
  }
}

void ModalSmall::render(int dt) {
  auto& draw = window->getDraw();
  draw.drawRect(style.x, style.y, style.width, style.height, props.backgroundColor);
  UiElement::render(dt);
}

} // namespace ui

export {

// --- from ui/layouts/ModalStandard.h ---
// IWYU pragma: keep

namespace ui {

// ModalStandard layout properties
struct ModalStandardProps {
  // For LayoutFit::CappedCentered (default), width/height are window dims.
  // For LayoutFit::FullBleed, width/height are the modal size as-is.
  int width = 0;
  int height = 0;
  LayoutFit layoutFit = LayoutFit::CappedCentered;
  SDL_Color contentBackgroundColor = Colors::White;
  bmin::String decorationSprite = "";
  bmin::String iconSprite;
  // Scales headerHeight (base 80) and the portrait that fits inside the top-left
  // OutsetRectangle well (headerHeight - 2 * outset border).
  float portraitScale = 1.f;
};

// ModalStandard layout - renders a modal with background, border, title, subtitle, close
// button, and children Uses Position, Size, Scale from BaseStyle
class ModalStandard : public UiElement {
private:
  ModalStandardProps props;

public:
  ModalStandard(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ModalStandard() override = default;

  // Setters and getters for layout-specific properties
  void setProps(const ModalStandardProps& _props);
  ModalStandardProps& getProps();
  const ModalStandardProps& getProps() const;

  void setTitleElement(UiElement* _titleElement);
  UiElement* getTitleElement();
  UiElement* getCloseButtonElement();

  const std::pair<int, int> getSubTitleDims();
  const std::pair<int, int> getSubTitleLocation();
  const std::pair<int, int> getContentDims();
  const std::pair<int, int> getContentLocation();

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

ModalStandard::ModalStandard(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void ModalStandard::setProps(const ModalStandardProps& _props) {
  props = _props;
  build();
}

ModalStandardProps& ModalStandard::getProps() { return props; }

const ModalStandardProps& ModalStandard::getProps() const { return props; }

const std::pair<int, int> ModalStandard::getSubTitleDims() {
  auto borderElement = dynamic_cast<BorderModalStandard*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  return borderElement->getSubTitleDims();
}

const std::pair<int, int> ModalStandard::getSubTitleLocation() {
  auto borderElement = dynamic_cast<BorderModalStandard*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  return borderElement->getSubTitleLocation();
}

const std::pair<int, int> ModalStandard::getContentDims() {
  auto borderElement = dynamic_cast<BorderModalStandard*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  return borderElement->getContentDims();
}

const std::pair<int, int> ModalStandard::getContentLocation() {
  auto borderElement = dynamic_cast<BorderModalStandard*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  return borderElement->getContentLocation();
}

void ModalStandard::build() {
  removeChildById("border");
  removeChildById("closeButton");
  removeChildById("headerIcon");

  if (props.width > 0 && props.height > 0) {
    if (props.layoutFit == LayoutFit::CappedCentered) {
      const auto rect = computeCappedCenteredRect(props.width, props.height);
      style.x = rect.x;
      style.y = rect.y;
      style.width = rect.width;
      style.height = rect.height;
    } else {
      style.width = props.width;
      style.height = props.height;
    }
  }

  // Create border element
  auto border = new BorderModalStandard(window, this);
  border->setId("border");
  border->setPos(style.x, style.y);
  border->setScale(style.scale);
  // portraitScale grows headerHeight; portrait fits the inner outset well.
  constexpr int kBaseHeaderHeight = 80;
  const float portraitScale = props.portraitScale > 0.f ? props.portraitScale : 1.f;
  const int headerHeight = static_cast<int>(kBaseHeaderHeight * portraitScale);
  const int outsetBorder = OutsetRectangleProps{}.borderSize; // 4 → inner well -8
  const int portraitFitSize = std::max(1, headerHeight - outsetBorder * 2);
  border->setProps(BorderModalSmallProps{
      .width = style.width,
      .height = style.height,
      .headerHeight = headerHeight,
      .iconSize = portraitFitSize,
      .borderWidth = 2,
  });

  // Insert border at the beginning
  addChild(border);

  // Get close button location before moving border
  auto [closeX, closeY] = border->getCloseButtonLocation();

  auto modalClose = new ButtonClose(window, this);
  modalClose->setId("closeButton");
  modalClose->setPos(closeX, closeY);
  modalClose->setScale(style.scale);
  ui::ButtonCloseProps modalCloseProps;
  modalCloseProps.closeType = ui::CloseType::MODAL;
  modalClose->setProps(modalCloseProps);
  addChild(modalClose);

  if (!props.iconSprite.empty()) {
    auto* border = dynamic_cast<BorderModalStandard*>(getChildById("border"));
    if (border != nullptr) {
      // drawSprite always uses the sprite's native w/h (params.w/h are ignored), so the
      // Quad texture must match the sprite or the image is clipped (e.g. 52px portraits
      // into a 32px texture looked shifted up-left).
      const auto& sprite =
          window->getStore().getSprite(bmin::toStringView(props.iconSprite));
      const int spriteW = std::max(1, sprite.w);
      const int spriteH = std::max(1, sprite.h);
      // Inner area of the top-left OutsetRectangle: headerHeight - 2 * borderSize.
      const int fitSize = border->getProps().iconSize;
      const float fitScale =
          static_cast<float>(fitSize) / static_cast<float>(std::max(spriteW, spriteH));
      const float drawScale = fitScale * style.scale;
      const int screenW = static_cast<int>(spriteW * drawScale);
      const int screenH = static_cast<int>(spriteH * drawScale);

      auto [centerX, centerY] = border->getIconSectionCenter();

      auto icon = new Quad(window, this);
      icon->setId("headerIcon");
      icon->setPos(centerX - screenW / 2, centerY - screenH / 2);
      icon->setScale(drawScale);
      icon->setProps(QuadProps{
          .width = spriteW,
          .height = spriteH,
          .bgSprite = props.iconSprite,
      });
      addChild(icon);
    }
  }

  // TODO decoration sprite
}

void ModalStandard::setTitleElement(UiElement* _titleElement) {
  removeChildById("title");
  auto borderElement = dynamic_cast<BorderModalStandard*>(getChildById("border"));
  // Add new title element
  if (_titleElement && borderElement) {
    auto [titleX, titleY] = borderElement->getTitleLocation();
    auto [titleWidth, titleHeight] = _titleElement->getDims();
    _titleElement->setPos(titleX, titleY - titleHeight / 2);
    _titleElement->setId("title");
    addChild(_titleElement);
  }
}

UiElement* ModalStandard::getTitleElement() { return getChildById("title"); }

UiElement* ModalStandard::getCloseButtonElement() { return getChildById("closeButton"); }

void ModalStandard::render(int dt) { UiElement::render(dt); }

} // namespace ui

export {

// --- from ui/layouts/InGameLayout.h ---
// IWYU pragma: keep

namespace ui {


enum class InGameBorderType { Wide, Narrow };

// InGameLayout layout properties
struct InGameLayoutProps {
  int width = 0;
  int height = 0;
  bmin::DynArray<state::WorldActionType> worldActionTypes;
  bmin::DynArray<ChCompactInfoProps> partyMembers;
  int selectedPartyMemberIndex = 0;
  float actionButtonScale = 1.f;
  InGameBorderType borderType = InGameBorderType::Wide;
};

// InGameLayout layout - renders an in-game layout with background, border, title,
// subtitle, and children Uses Position, Size, Scale from BaseStyle
class InGameLayout : public UiElement {
private:
  InGameLayoutProps props;

  void applyTitleLayout(UiElement* titleElement, BorderInGame* border);
  void buildActionButtons(const std::pair<int, int>& actionButtonsAreaLocation,
                          int actionButtonsQuadWidth,
                          int actionButtonsQuadHeight,
                          Quad* actionButtonsQuad);
  void buildChList(const std::pair<int, int>& chListLocation);

public:
  InGameLayout(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~InGameLayout() override = default;

  void setProps(const InGameLayoutProps& _props);
  InGameLayoutProps& getProps();
  const InGameLayoutProps& getProps() const;

  void setTitleElement(UiElement* _titleElement);
  UiElement* getTitleElement();
  const std::pair<int, int> getWorldDims();
  const std::pair<int, int> getWorldLocation();
  const std::pair<int, int> getChListLocation();

  // Modal-style X + mode label in the bottom-left of the map area.
  void setActionModeCancelVisible(bool visible, const bmin::String& modeLabel = {});

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverCancelWorldActionMode.hpp ---
namespace ui {

class ObserverCancelWorldActionMode : public UiEventObserver {
  state::StateManager* stateManager;

public:
  explicit ObserverCancelWorldActionMode(state::StateManager* _stateManager)
      : stateManager(_stateManager) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override;
};

} // namespace ui

// --- from ui/observers/ObserverWorldAction.hpp ---
namespace ui {

class ObserverWorldAction : public UiEventObserver {
  state::StateManager* stateManager;
  state::WorldActionType worldActionType;
  sdl2w::Window* window;

public:
  ObserverWorldAction(state::StateManager* _stateManager,
                      state::WorldActionType _worldActionType,
                      sdl2w::Window* _window)
      : stateManager(_stateManager),
        worldActionType(_worldActionType),
        window(_window) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

// --- from ui/observers/ObserverSetSelectedPartyMemberId.hpp ---
namespace ui {

class ObserverSetSelectedPartyMemberId : public ui::UiEventObserver,
                                         public state::StateManagerInterface {
  bmin::String partyMemberId;

public:
  explicit ObserverSetSelectedPartyMemberId(bmin::String _partyMemberId)
      : partyMemberId(std::move(_partyMemberId)) {}

  void onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) override;
};

} // namespace ui

} // export

namespace ui {

InGameLayout::InGameLayout(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void InGameLayout::setProps(const InGameLayoutProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

InGameLayoutProps& InGameLayout::getProps() { return props; }

const InGameLayoutProps& InGameLayout::getProps() const { return props; }

void InGameLayout::applyTitleLayout(UiElement* titleElement, BorderInGame* border) {
  if (!titleElement || !border) {
    return;
  }

  auto [titleX, titleY] = border->getTitleLocation();
  auto [titleWidth, titleBarHeight] = border->getTitleDims();
  titleElement->setPos(titleX, titleY);
  titleElement->setScale(style.scale);
  if (auto* titleBar = dynamic_cast<InGameTitleBar*>(titleElement)) {
    auto titleProps = titleBar->getProps();
    titleProps.width = static_cast<int>(titleWidth / style.scale);
    titleProps.height = static_cast<int>(titleBarHeight / style.scale);
    titleBar->setProps(titleProps);
  } else {
    titleElement->build();
  }
}

void InGameLayout::buildActionButtons(const std::pair<int, int>& actionButtonsAreaLocation,
                                      int actionButtonsQuadWidth,
                                      int actionButtonsQuadHeight,
                                      Quad* actionButtonsQuad) {
  actionButtonsQuad->setPos(actionButtonsAreaLocation.first,
                            actionButtonsAreaLocation.second);
  actionButtonsQuad->setScale(style.scale);
  actionButtonsQuad->setProps(QuadProps{
      .width = actionButtonsQuadWidth,
      .height = actionButtonsQuadHeight,
  });

  auto buttonWidthScaled =
      static_cast<int>(BorderInGame::ACTION_BUTTON_SIZE * props.actionButtonScale);
  auto buttonSmallIndex = 0;
  auto xAgg = 0;
  auto xAdditionalOffset = 0;
  for (int i = 0; i < static_cast<int>(props.worldActionTypes.size()); i++) {
    if (i == 1) {
      xAdditionalOffset += buttonWidthScaled / 4;
    }
    auto worldActionType = props.worldActionTypes[i];
    auto button =
        new ButtonWorldAction(actionButtonsQuad->getWindow(), actionButtonsQuad);
    auto isSmall = ButtonWorldAction::checkIfWorldActionButtonIsSmall(worldActionType);

    int buttonX = xAgg + xAdditionalOffset;
    int buttonY = 0;

    xAgg += buttonWidthScaled;
    if (isSmall) {
      if (buttonSmallIndex == 0) {
        xAgg -= buttonWidthScaled;
        buttonSmallIndex = 1;
      } else {
        buttonY += buttonWidthScaled / 2;
        buttonSmallIndex = 0;
      }
    }

    button->setId("worldActionButton_" + bmin::toString(i));
    button->setPos(buttonX, buttonY);
    button->setScale(props.actionButtonScale);
    button->setProps(ButtonWorldActionProps{worldActionType});
    actionButtonsQuad->addChild(button);
  }

  actionButtonsQuad->build();
}

void InGameLayout::buildChList(const std::pair<int, int>& chListLocation) {
  if (props.partyMembers.empty()) {
    return;
  }

  auto [chListX, chListY] = chListLocation;

  if (props.borderType == InGameBorderType::Wide) {
    auto chList = new ListChCompactInfoVertical(window, this);
    chList->setId("chList");
    chList->setPos(chListX, chListY);
    chList->setScale(style.scale);
    chList->setProps(ListChCompactInfoVerticalProps{
        .entries = props.partyMembers,
        .selectedIndex = props.selectedPartyMemberIndex,
        .lineGap = 6,
    });
    addChild(chList);
  } else {
    auto chList = new ListChCompactInfoHorizontal(window, this);
    chList->setId("chList");
    chList->setPos(chListX, chListY);
    chList->setScale(style.scale);
    chList->setProps(ListChCompactInfoHorizontalProps{
        .entries = props.partyMembers,
        .selectedIndex = props.selectedPartyMemberIndex,
        .lineGap = 6,
    });
    addChild(chList);
  }
}

void InGameLayout::setTitleElement(UiElement* _titleElement) {
  removeChildById("title");
  if (!_titleElement) {
    return;
  }

  applyTitleLayout(_titleElement,
                   dynamic_cast<BorderInGame*>(getChildById("border")));

  _titleElement->setId("title");
  addChild(_titleElement);
}

UiElement* InGameLayout::getTitleElement() { return getChildById("title"); }

void InGameLayout::setActionModeCancelVisible(bool visible,
                                              const bmin::String& modeLabel) {
  removeChildById("actionModeCancel");
  removeChildById("actionModeLabelBg");
  removeChildById("actionModeLabel");
  if (!visible) {
    return;
  }

  constexpr int kPad = 4;
  constexpr int kLabelGap = 8;
  constexpr int kLabelPad = 5;
  auto [worldX, worldY] = getWorldLocation();
  auto [worldW, worldH] = getWorldDims();

  auto* cancelButton = new ButtonClose(window, this);
  cancelButton->setId("actionModeCancel");
  cancelButton->setScale(style.scale);
  const int buttonSize = static_cast<int>(ButtonClose::closeButtonSize * style.scale);
  const int buttonX = worldX + kPad;
  const int buttonY = worldY + worldH - buttonSize - kPad;
  cancelButton->setPos(buttonX, buttonY);
  cancelButton->setProps(ButtonCloseProps{
      .closeType = CloseType::MODAL,
  });
  addChild(cancelButton);

  if (!modeLabel.empty()) {
    const int labelX = buttonX + buttonSize + kLabelGap;
    const int labelY = buttonY + buttonSize / 2;

    auto* label = new TextLine(window, this);
    label->setId("actionModeLabel");
    label->setScale(style.scale);
    label->setPos(labelX, labelY);
    label->setProps(TextLineProps{
        .textBlocks = {{.text = modeLabel}},
        .fontFamily = FontFamily::TEXT,
        .fontSize = sdl2w::TEXT_SIZE_16,
        .fontColor = Colors::White,
        .textAlign = TextAlign::LEFT_CENTER,
    });

    const auto [labelW, labelH] = label->getDims();
    const int padScaled = static_cast<int>(kLabelPad * style.scale);

    auto* labelBg = new OutsetRectangle(window, this);
    labelBg->setId("actionModeLabelBg");
    labelBg->setPos(labelX - padScaled, labelY - labelH / 2 - padScaled);
    labelBg->setScale(1.f);
    labelBg->setProps(OutsetRectangleProps{
        .width = labelW + padScaled * 2,
        .height = labelH + padScaled * 2,
    });
    addChild(labelBg);
    addChild(label);
  }
}

const std::pair<int, int> InGameLayout::getWorldDims() {
  auto* border = dynamic_cast<BorderInGame*>(getChildById("border"));
  if (border) {
    return border->getContentDims();
  }
  return {style.width, style.height};
}

const std::pair<int, int> InGameLayout::getWorldLocation() {
  auto* border = dynamic_cast<BorderInGame*>(getChildById("border"));
  if (border) {
    return border->getContentAreaLocation();
  }
  return {style.x, style.y};
}

const std::pair<int, int> InGameLayout::getChListLocation() {
  auto* border = dynamic_cast<BorderInGame*>(getChildById("border"));
  if (border) {
    return border->getPartyMemberAreaLocation();
  }
  return {0, 0};
}

void InGameLayout::build() {
  removeChildById("border");
  removeChildById("actionButtons");
  removeChildById("chList");
  removeChildById("actionModeCancel");
  removeChildById("actionModeLabelBg");
  removeChildById("actionModeLabel");

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  std::pair<int, int> actionButtonsAreaLocation;
  int actionButtonsQuadWidth = 0;
  int actionButtonsQuadHeight = 0;

  if (props.borderType == InGameBorderType::Wide) {
    auto wideBorder = new BorderInGameWide(window, this);
    wideBorder->setId("border");
    wideBorder->setPos(style.x, style.y);
    wideBorder->setScale(style.scale);
    auto borderProps = BorderInGameWideProps{};
    borderProps.width = style.width;
    borderProps.height = style.height;
    borderProps.actionButtonsScale = props.actionButtonScale;
    wideBorder->setProps(borderProps);
    actionButtonsAreaLocation = wideBorder->getActionButtonsAreaLocation();
    actionButtonsQuadWidth = style.width - borderProps.leftBorderWidth -
                             borderProps.partyMemberAreaWidth -
                             2 * borderProps.outsetBorderSize;
    actionButtonsQuadHeight = BorderInGame::ACTION_BUTTON_SIZE * 2 * borderProps.actionButtonsScale +
                              borderProps.outsetBorderSize * 2;
    children.insert(children.begin(), bmin::UniquePtr<UiElement>(wideBorder));
  } else {
    auto narrowBorder = new BorderInGameNarrow(window, this);
    narrowBorder->setId("border");
    narrowBorder->setPos(style.x, style.y);
    narrowBorder->setScale(style.scale);
    auto borderProps = BorderInGameNarrowProps{};
    borderProps.width = style.width;
    borderProps.height = style.height;
    borderProps.actionButtonsScale = props.actionButtonScale;
    narrowBorder->setProps(borderProps);
    actionButtonsAreaLocation = narrowBorder->getActionButtonsAreaLocation();
    actionButtonsQuadWidth =
        style.width - borderProps.sideBorderWidth * 2 - 2 * borderProps.outsetBorderSize;
    actionButtonsQuadHeight = BorderInGame::ACTION_BUTTON_SIZE * 2 * borderProps.actionButtonsScale +
                              borderProps.outsetBorderSize * 2;
    children.insert(children.begin(), bmin::UniquePtr<UiElement>(narrowBorder));
  }

  auto actionButtonsQuad = new Quad(window, this);
  actionButtonsQuad->setId("actionButtons");
  addChild(actionButtonsQuad);
  buildActionButtons(actionButtonsAreaLocation,
                     actionButtonsQuadWidth,
                     actionButtonsQuadHeight,
                     actionButtonsQuad);

  buildChList(getChListLocation());

  if (auto* titleElement = getTitleElement()) {
    applyTitleLayout(titleElement,
                     dynamic_cast<BorderInGame*>(getChildById("border")));
  }
}

void InGameLayout::render(int dt) { UiElement::render(dt); }

} // namespace ui

namespace ui {

void ObserverCancelWorldActionMode::onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) {
    if (stateManager) {
      cancelCurrentWorldActionMode(*stateManager);
    }
  }

void ObserverWorldAction::onClick(int mouseX, int mouseY, int button) {
    if (stateManager) {
      activateWorldAction(*stateManager, worldActionType, window);
    }
  }

void ObserverSetSelectedPartyMemberId::onClick(int /*mouseX*/, int /*mouseY*/, int /*button*/) {
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    // Party member switching is locked while combat is active.
    if (stateManager->getState().world.combat.active) {
      return;
    }
    LOG(INFO) << "ObserverSetSelectedPartyMemberId::onClick id=" << partyMemberId
              << LOG_ENDL;
    stateManager->enqueueAction(
        stateManager->getActionData(),
        state::actions::setSelectedPartyMemberId(partyMemberId),
        0);
  }

} // namespace ui
