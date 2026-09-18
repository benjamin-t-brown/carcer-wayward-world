#include "ui/pages/specialEventViewMapping.h"
#include "sdl2w/L10n.h"
#include "ui/colors.hpp"

namespace ui {

PageTalkChoiceItem
SpecialEventViewMapping::mapChoice(const game::SpecialEventChoiceView& choice) {
  auto item = PageTalkChoiceItem{};
  item.text = choice.text;
  item.prefixText = choice.prefix;
  item.previouslyChosen = choice.previouslyChosen;
  return item;
}

bmin::String
SpecialEventViewMapping::noticeText(const game::SpecialEventTranscriptEntry& entry) {
  if (entry.kind == game::SpecialEventTranscriptKind::JournalNotice) {
    return TRANSLATE("Your journal has been updated.");
  }
  return TRANSLATE("You have received ") + entry.text + ".";
}

bool SpecialEventViewMapping::isNotice(game::SpecialEventTranscriptKind kind) {
  return kind == game::SpecialEventTranscriptKind::JournalNotice ||
         kind == game::SpecialEventTranscriptKind::ItemReceived;
}

TextBlock SpecialEventViewMapping::mapEntry(
    const game::SpecialEventTranscriptEntry& entry,
    bool historySeparator,
    bool leadingBlankLine) {
  auto block = TextBlock{};
  switch (entry.kind) {
  case game::SpecialEventTranscriptKind::PlayerChoice:
    block.text = bmin::String("> ") + entry.text + "\n\n";
    block.fontColor = Colors::DarkBlue;
    return block;
  case game::SpecialEventTranscriptKind::JournalNotice:
  case game::SpecialEventTranscriptKind::ItemReceived: {
    const auto notice = noticeText(entry);
    if (historySeparator) {
      block.text = notice + "\n\n";
    } else if (leadingBlankLine) {
      block.text = bmin::String("\n\n") + notice;
    } else {
      block.text = notice;
    }
    block.fontColor = Colors::Grey;
    return block;
  }
  case game::SpecialEventTranscriptKind::Dialogue:
  default:
    block.text = historySeparator ? entry.text + "\n\n" : entry.text;
    return block;
  }
}

PageTalkChoiceProps
SpecialEventViewMapping::toTalkProps(const game::SpecialEventView& view,
                                     int width,
                                     int height) {
  auto props = PageTalkChoiceProps{};
  props.width = width;
  props.height = height;
  props.title = view.title;
  props.portraitSpriteName = view.portraitSpriteName;
  props.portraitScale = view.portraitScale;
  props.pinFromBlockIndex = view.pinFromBlockIndex;
  props.showContinue = view.showContinue;
  for (const auto& entry : view.history) {
    props.textBlocks.pushBack(mapEntry(entry, true, false));
  }
  for (const auto& entry : view.current) {
    props.textBlocks.pushBack(mapEntry(entry, false, false));
  }
  for (const auto& choice : view.choices) {
    props.choices.pushBack(mapChoice(choice));
  }
  return props;
}

PageModalEventProps
SpecialEventViewMapping::toModalProps(const game::SpecialEventView& view,
                                      int width,
                                      int height) {
  auto props = PageModalEventProps{};
  props.width = width;
  props.height = height;
  props.title = view.title;
  props.showContinueButton = view.showContinue;

  auto hasDialogue = bool{false};
  for (const auto& entry : view.current) {
    if (entry.kind == game::SpecialEventTranscriptKind::Dialogue &&
        !entry.text.empty()) {
      hasDialogue = true;
      break;
    }
  }

  auto usedLeadingBlank = bool{false};
  for (const auto& entry : view.current) {
    const auto leadingBlankLine =
        isNotice(entry.kind) && hasDialogue && !usedLeadingBlank;
    if (leadingBlankLine) {
      usedLeadingBlank = true;
    }
    props.textBlocks.pushBack(mapEntry(entry, false, leadingBlankLine));
  }
  for (const auto& choice : view.choices) {
    props.choices.pushBack(mapChoice(choice));
  }
  return props;
}

} // namespace ui
