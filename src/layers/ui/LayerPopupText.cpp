#include "LayerPopupText.h"
#include "lib/sdl2w/Logger.h"
#include "ui/colors.h"
#include "ui/components/borders/BorderDropShadow.h"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/elements/Quad.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/TextParagraph.h"
#include "ui/elements/buttons/ButtonClose.h"
#include "ui/observers/ObserverRemoveLayer.hpp"
#include <algorithm>

namespace layers {

namespace {

constexpr int POPUP_WIDTH = 300;
constexpr int PADDING = 8;
constexpr int CLOSE_BUTTON_PADDING = 4;

} // namespace

LayerPopupText::LayerPopupText(sdl2w::Window* _window,
                               bmin::String title,
                               bmin::String text)
    : Layer(_window, LAYER_ID) {

  if (!assertInterfaces()) {
    remove();
    return;
  }

  if (text.empty()) {
    LOG(ERROR) << "LayerPopupText: text is empty" << LOG_ENDL;
    remove();
    return;
  }

  auto [windowWidth, windowHeight] = window->getDims();
  const int closeButtonSize = ui::ButtonClose::closeButtonSize;
  const int headerY = CLOSE_BUTTON_PADDING;
  const int titleWidth =
      POPUP_WIDTH - 2 * PADDING - closeButtonSize - CLOSE_BUTTON_PADDING;
  const int bodyWidth = POPUP_WIDTH - 2 * PADDING;

  auto* titleLine = new ui::TextLine(window, nullptr);
  titleLine->setId("title");
  auto& titleStyle = titleLine->getStyle();
  titleStyle.x = PADDING;
  titleStyle.y = headerY + closeButtonSize / 2;
  titleStyle.width = titleWidth;
  titleStyle.scale = 1.f;
  ui::setBaseFontConfig(titleStyle, ui::BaseFontConfig::MODAL_TITLE);
  titleStyle.fontColor = ui::Colors::Black;
  titleStyle.textAlign = ui::TextAlign::LEFT_CENTER;
  ui::TextLineProps titleProps;
  titleProps.textBlocks.pushBack({.text = title});
  titleLine->setProps(titleProps);
  auto [_, titleHeight] = titleLine->calculateTextDims();
  const int headerHeight = std::max(titleHeight, closeButtonSize);
  int contentY = headerY + headerHeight + PADDING;

  auto* paragraph = new ui::TextParagraph(window, nullptr);
  paragraph->setId("helpText");
  auto& paragraphStyle = paragraph->getStyle();
  paragraphStyle.x = PADDING;
  paragraphStyle.y = contentY;
  paragraphStyle.width = bodyWidth;
  paragraphStyle.scale = 1.f;
  ui::setBaseFontConfig(paragraphStyle, ui::BaseFontConfig::MODAL_TEXT);
  paragraphStyle.fontColor = ui::Colors::Black;
  paragraphStyle.textAlign = ui::TextAlign::LEFT_TOP;
  paragraphStyle.lineSpacing = 0;
  ui::TextParagraphProps paragraphProps;
  paragraphProps.textBlocks.pushBack({.text = text});
  paragraphProps.padding = 0;
  paragraph->setProps(paragraphProps);
  auto [__, messageHeight] = paragraph->getDims();
  contentY += messageHeight;

  const int popupHeight = contentY + PADDING;
  const int popupX = (windowWidth - POPUP_WIDTH) / 2;
  const int popupY = (windowHeight - popupHeight) / 2;

  auto* root = new ui::UiElement(window, nullptr);
  root->setId("popupTextRoot");
  auto& rootStyle = root->getStyle();
  rootStyle.x = 0;
  rootStyle.y = 0;
  rootStyle.width = windowWidth;
  rootStyle.height = windowHeight;
  rootStyle.scale = 1.f;

  auto* backdrop = new ui::Quad(window, root);
  backdrop->setId("backdrop");
  auto& backdropStyle = backdrop->getStyle();
  backdropStyle.x = 0;
  backdropStyle.y = 0;
  backdropStyle.width = windowWidth;
  backdropStyle.height = windowHeight;
  backdropStyle.scale = 1.f;
  backdrop->setProps(ui::QuadProps{.bgColor = ui::Colors::Transparent});
  backdrop->addEventObserver(new ui::ObserverRemoveLayer(LAYER_ID));
  root->addChild(backdrop);

  auto* border = new ui::BorderDropShadow(window, root);
  border->setId("border");
  auto& borderStyle = border->getStyle();
  borderStyle.x = popupX;
  borderStyle.y = popupY;
  borderStyle.width = POPUP_WIDTH;
  borderStyle.height = popupHeight;
  borderStyle.scale = 1.f;
  border->setProps(ui::BorderDropShadowProps{
      .backgroundColor = ui::Colors::White,
      .shadowColor = ui::Colors::Black,
      .borderSize = 2,
  });
  border->addChild(titleLine);
  border->addChild(paragraph);
  root->addChild(border);

  auto* closeButton = new ui::ButtonClose(window, root);
  closeButton->setId("closeButton");
  auto& closeStyle = closeButton->getStyle();
  closeStyle.x = popupX + POPUP_WIDTH - closeButtonSize - CLOSE_BUTTON_PADDING;
  closeStyle.y = popupY + CLOSE_BUTTON_PADDING;
  closeStyle.width = closeButtonSize;
  closeStyle.height = closeButtonSize;
  closeStyle.scale = 1.f;
  closeButton->setProps(ui::ButtonCloseProps{.closeType = ui::CloseType::POPUP});
  closeButton->addEventObserver(new ui::ObserverRemoveLayer(LAYER_ID));
  root->addChild(closeButton);

  addUiElement(root);

  auto* floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);
}

void LayerPopupText::update(int deltaTime) { Layer::update(deltaTime); }

void LayerPopupText::render(int deltaTime) { Layer::render(deltaTime); }

} // namespace layers
