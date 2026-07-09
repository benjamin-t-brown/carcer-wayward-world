#include "LayerPopupText.h"
#include "sdl2w/Logger.h"
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
  titleLine->setPos(PADDING, headerY + closeButtonSize / 2);
  titleLine->setScale(1.f);
  ui::TextFontProps titleFont;
  ui::setBaseFontConfig(titleFont, ui::BaseFontConfig::MODAL_TITLE);
  ui::TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = titleFont.fontSize;
  titleProps.fontColor = ui::Colors::Black;
  titleProps.textAlign = ui::TextAlign::LEFT_CENTER;
  titleProps.textBlocks.pushBack({.text = title});
  titleLine->setProps(titleProps);
  auto [_, titleHeight] = titleLine->calculateTextDims();
  const int headerHeight = std::max(titleHeight, closeButtonSize);
  int contentY = headerY + headerHeight + PADDING;

  auto* paragraph = new ui::TextParagraph(window, nullptr);
  paragraph->setId("helpText");
  paragraph->setPos(PADDING, contentY);
  paragraph->setScale(1.f);
  ui::TextFontProps paragraphFont;
  ui::setBaseFontConfig(paragraphFont, ui::BaseFontConfig::MODAL_TEXT);
  ui::TextParagraphProps paragraphProps;
  paragraphProps.width = bodyWidth;
  paragraphProps.fontFamily = paragraphFont.fontFamily;
  paragraphProps.fontSize = paragraphFont.fontSize;
  paragraphProps.fontColor = ui::Colors::Black;
  paragraphProps.textAlign = ui::TextAlign::LEFT_TOP;
  paragraphProps.lineSpacing = 0;
  paragraphProps.padding = 0;
  paragraphProps.textBlocks.pushBack({.text = text});
  paragraph->setProps(paragraphProps);
  auto [__, messageHeight] = paragraph->getDims();
  contentY += messageHeight;

  const int popupHeight = contentY + PADDING;
  const int popupX = (windowWidth - POPUP_WIDTH) / 2;
  const int popupY = (windowHeight - popupHeight) / 2;

  auto* root = new ui::UiElement(window, nullptr);
  root->setId("popupTextRoot");
  root->setPos(0, 0);
  root->setScale(1.f);

  auto* backdrop = new ui::Quad(window, root);
  backdrop->setId("backdrop");
  backdrop->setPos(0, 0);
  backdrop->setScale(1.f);
  backdrop->setProps(ui::QuadProps{
      .width = windowWidth,
      .height = windowHeight,
      .bgColor = ui::Colors::Transparent,
  });
  backdrop->addEventObserver(new ui::ObserverRemoveLayer(LAYER_ID));
  root->addChild(backdrop);

  auto* border = new ui::BorderDropShadow(window, root);
  border->setId("border");
  border->setPos(popupX, popupY);
  border->setScale(1.f);
  border->setProps(ui::BorderDropShadowProps{
      .width = POPUP_WIDTH,
      .height = popupHeight,
      .backgroundColor = ui::Colors::White,
      .shadowColor = ui::Colors::Black,
      .borderSize = 2,
  });
  border->addChild(titleLine);
  border->addChild(paragraph);
  root->addChild(border);

  auto* closeButton = new ui::ButtonClose(window, root);
  closeButton->setId("closeButton");
  closeButton->setPos(popupX + POPUP_WIDTH - closeButtonSize - CLOSE_BUTTON_PADDING,
                      popupY + CLOSE_BUTTON_PADDING);
  closeButton->setScale(1.f);
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
