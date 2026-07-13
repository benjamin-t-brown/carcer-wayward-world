#include "PageTalkChoice.h"
#include "ui/colors.h"
#include "ui/components/borders/BorderModalStandard.h"
#include "ui/elements/OutsetRectangle.h"
#include "ui/elements/Quad.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/TextParagraph.h"
#include "ui/elements/buttons/ButtonTextWrap.h"
#include "ui/layouts/ModalStandard.h"
#include <algorithm>

namespace ui {

PageTalkChoice::PageTalkChoice(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Page doesn't need special initialization
}

void PageTalkChoice::setProps(const PageTalkChoiceProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

PageTalkChoiceProps& PageTalkChoice::getProps() { return props; }

const PageTalkChoiceProps& PageTalkChoice::getProps() const { return props; }

const std::pair<int, int> PageTalkChoice::getDims() const {
  if (children.empty()) {
    return {style.width, style.height};
  }
  return children[0]->getDims();
}

void PageTalkChoice::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  // Create ModalStandard layout
  auto modal = new ModalStandard(window, this);
  modal->setId("modal");
  modal->setPos(style.x, style.y);
  modal->setScale(style.scale);
  modal->setProps(ModalStandardProps{
      .width = style.width,
      .height = style.height,
      .iconSprite = props.portraitSpriteName,
  });

  children.pushBack(bmin::UniquePtr<UiElement>(modal));

  if (!props.portraitSpriteName.empty()) {
    if (auto* border = dynamic_cast<BorderModalStandard*>(modal->getChildById("border"))) {
      auto [iconX, iconY] = border->getIconBorderLocation();
      const int iconSize = border->getProps().iconSize;
      auto iconBg = bmin::makeUnique<Quad>(window, modal);
      iconBg->setId("headerIconBg");
      iconBg->setPos(iconX, iconY);
      iconBg->setScale(style.scale);
      iconBg->setProps(QuadProps{
          .width = iconSize,
          .height = iconSize,
          .bgColor = Colors::OffWhite,
      });

      auto& modalChildren = modal->getChildren();
      const auto insertBefore = std::find_if(modalChildren.begin(),
                                             modalChildren.end(),
                                             [](const bmin::UniquePtr<UiElement>& child) {
                                               return child->getId() == "headerIcon";
                                             });
      if (insertBefore != modalChildren.end()) {
        modalChildren.insert(insertBefore, bmin::UniquePtr<UiElement>(iconBg.release()));
      } else {
        modal->addChild(iconBg.release());
      }
    }
  }

  auto [scaledContentW, scaledContentH] = modal->getContentDims();
  auto [contentX, contentY] = modal->getContentLocation();

  auto choiceSectionHeight = props.choiceAreaHeight;
  auto textSectionHeight =
      (scaledContentH / style.scale - BorderModalStandard::BOTTOM_BORDER_HEIGHT -
       choiceSectionHeight);
  auto borderHeight = BorderModalStandard::BOTTOM_BORDER_HEIGHT;
  auto scrollBarWidth = 32;

  // Create title element
  auto title = new TextLine(window, this);
  TextFontProps titleFont;
  setBaseFontConfig(titleFont, BaseFontConfig::MODAL_TITLE);
  TextLineProps titleProps;
  titleProps.fontFamily = titleFont.fontFamily;
  titleProps.fontSize = sdl2w::TEXT_SIZE_24;
  titleProps.fontColor = titleFont.fontColor;
  titleProps.textAlign = TextAlign::LEFT_TOP;
  TextBlock titleBlock;
  titleBlock.text = props.title;
  titleProps.textBlocks.pushBack(titleBlock);
  title->setProps(titleProps);
  modal->setTitleElement(title);

  // Create SectionScrollable for content area
  auto textSection = new SectionScrollable(window, this);
  textSection->setId("textSection");
  textSection->setPos(contentX, contentY);
  textSection->setScale(style.scale);
  textSection->setProps(SectionScrollableProps{
      .width = static_cast<int>(scaledContentW / style.scale),
      .height = static_cast<int>(textSectionHeight),
      .scrollBarWidth = scrollBarWidth,
  });
  addChild(textSection);
  auto [textScrollableContentWidthScaled, textViewportHeightScaled] =
      textSection->getContentDims();

  TextFontProps textFont;
  setBaseFontConfig(textFont, BaseFontConfig::MODAL_CHOICE_TEXT);

  const int pinFrom = std::clamp(props.pinFromBlockIndex,
                                 0,
                                 static_cast<int>(props.textBlocks.size()));
  bmin::DynArray<TextBlock> historyBlocks;
  bmin::DynArray<TextBlock> currentBlocks;
  for (int i = 0; i < static_cast<int>(props.textBlocks.size()); i++) {
    if (i < pinFrom) {
      historyBlocks.pushBack(props.textBlocks[i]);
    } else {
      currentBlocks.pushBack(props.textBlocks[i]);
    }
  }

  int contentYOffset = 0;
  int historyHeightScaled = 0;

  auto addParagraph = [&](const bmin::DynArray<TextBlock>& blocks,
                          const bmin::String& id) -> TextParagraph* {
    if (blocks.empty()) {
      return nullptr;
    }
    auto* paragraph = new TextParagraph(window, textSection);
    paragraph->setId(id);
    paragraph->setPos(0, contentYOffset);
    paragraph->setScale(1.f);
    paragraph->setProps(TextParagraphProps{
        .textBlocks = blocks,
        .width = textScrollableContentWidthScaled,
        .bgColor = Colors::OffWhite,
        .padding = 4,
        .lineSpacing = 0,
        .fontFamily = textFont.fontFamily,
        .fontSize = textFont.fontSize,
        .fontColor = textFont.fontColor,
    });
    textSection->addChild(paragraph);
    const int height = paragraph->getDims().second;
    contentYOffset += height;
    return paragraph;
  };

  if (auto* historyParagraph = addParagraph(historyBlocks, "textBlocksHistory")) {
    historyHeightScaled = historyParagraph->getDims().second;
  }

  int currentHeightScaled = 0;
  if (auto* currentParagraph = addParagraph(currentBlocks, "textBlocks")) {
    currentHeightScaled = currentParagraph->getDims().second;
  }

  // Pad so the pinned (current) dialogue can sit at the top of the viewport.
  if (currentHeightScaled > 0) {
    const int padHeightScaled =
        std::max(0, textViewportHeightScaled - currentHeightScaled);
    if (padHeightScaled > 0) {
      auto* spacer = new Quad(window, textSection);
      spacer->setId("textBottomPad");
      spacer->setPos(0, contentYOffset);
      spacer->setScale(1.f);
      spacer->setProps(QuadProps{
          .width = textScrollableContentWidthScaled,
          .height = padHeightScaled,
          .bgColor = Colors::OffWhite,
      });
      textSection->addChild(spacer);
    }
  }

  textSection->build();
  textSection->scrollTo(historyHeightScaled);

  auto sepBorder = new OutsetRectangle(window, this);
  sepBorder->setPos(contentX, contentY + textSectionHeight * style.scale);
  sepBorder->setScale(style.scale);
  sepBorder->setProps(OutsetRectangleProps{
      .width = static_cast<int>(scaledContentW / style.scale),
      .height = 10,
  });
  addChild(sepBorder);

  auto choiceSection = new SectionScrollable(window, this);
  choiceSection->setId("choiceSection");
  choiceSection->setPos(contentX,
                        contentY + (textSectionHeight + borderHeight) * style.scale);
  choiceSection->setScale(style.scale);
  choiceSection->setProps(SectionScrollableProps{
      .width = static_cast<int>(scaledContentW / style.scale),
      .height = choiceSectionHeight,
      .scrollBarWidth = scrollBarWidth,
      .indicatorHeight = 0,
  });
  addChild(choiceSection);

  // Create choices (setPos before setProps so ButtonTextWrap builds text at the right offset)
  auto choiceYOffset = 0;
  for (int i = 0; i < static_cast<int>(props.choices.size()); i++) {
    auto choiceButton = new ButtonTextWrap(window, choiceSection);
    choiceButton->setId("choice" + bmin::toString(i));
    TextFontProps choiceFont;
    setBaseFontConfig(choiceFont, BaseFontConfig::MODAL_CHOICE_TEXT);
    ui::ButtonTextWrapProps choiceButtonProps;
    const bmin::String& prefixText = props.choices[i].prefixText;
    choiceButtonProps.text =
        bmin::toString(i + 1) + ". " +
        ((prefixText.empty() ? props.choices[i].text
                             : prefixText + " " + props.choices[i].text));
    choiceButtonProps.width =
        scaledContentW - 8 * style.scale - scrollBarWidth * style.scale;
    choiceButtonProps.isSelected = false;
    choiceButtonProps.fontFamily = choiceFont.fontFamily;
    choiceButtonProps.fontSize = choiceFont.fontSize;
    choiceButtonProps.fontColor =
        props.choices[i].previouslyChosen ? Colors::Grey : Colors::DarkBlue;
    choiceButton->setScale(1.f);
    choiceButton->setPos(4 * style.scale, choiceYOffset);
    choiceButton->setProps(choiceButtonProps);
    auto [choiceWidth, choiceHeight] = choiceButton->getDims();
    choiceSection->addChild(choiceButton);
    choiceYOffset += choiceHeight;
  }

  choiceSection->build();
}

void PageTalkChoice::render(int dt) { UiElement::render(dt); }

} // namespace ui
