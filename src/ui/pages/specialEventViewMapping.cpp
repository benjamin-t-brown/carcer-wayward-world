#include "ui/pages/specialEventViewMapping.h"
#include "sdl2w/L10n.h"
#include "ui/colors.hpp"
#include <cstdlib>

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
  if (entry.kind == game::SpecialEventTranscriptKind::CoinsModified) {
    const int delta = entry.text.empty() ? 0 : std::atoi(entry.text.cStr());
    const int absDelta = delta < 0 ? -delta : delta;
    const bmin::String noun = absDelta == 1 ? TRANSLATE("coin") : TRANSLATE("coins");
    if (delta > 0) {
      return TRANSLATE("You have received ") + bmin::toString(absDelta) + " " + noun +
             ".";
    }
    if (delta < 0) {
      return TRANSLATE("You have lost ") + bmin::toString(absDelta) + " " + noun + ".";
    }
    return {};
  }
  if (entry.kind == game::SpecialEventTranscriptKind::ExperienceGained) {
    const int delta = entry.text.empty() ? 0 : std::atoi(entry.text.cStr());
    const int absDelta = delta < 0 ? -delta : delta;
    if (delta == 0) {
      return {};
    }
    const bmin::String prefix =
        delta > 0 ? TRANSLATE("You have received ") : TRANSLATE("You have lost ");
    return prefix + bmin::toString(absDelta) + " " + TRANSLATE("experience") + ".";
  }
  return TRANSLATE("You have received ") + entry.text + ".";
}

bool SpecialEventViewMapping::isNotice(game::SpecialEventTranscriptKind kind) {
  return kind == game::SpecialEventTranscriptKind::JournalNotice ||
         kind == game::SpecialEventTranscriptKind::ItemReceived ||
         kind == game::SpecialEventTranscriptKind::CoinsModified ||
         kind == game::SpecialEventTranscriptKind::ExperienceGained;
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
  case game::SpecialEventTranscriptKind::ItemReceived:
  case game::SpecialEventTranscriptKind::CoinsModified:
  case game::SpecialEventTranscriptKind::ExperienceGained: {
    const auto notice = noticeText(entry);
    if (notice.empty()) {
      return block;
    }
    // One notice per line. History keeps a blank line after; live/current notices
    // trail with a single newline so consecutive item grants don't run together.
    if (historySeparator) {
      block.text = notice + "\n\n";
    } else if (leadingBlankLine) {
      block.text = bmin::String("\n\n") + notice + "\n";
    } else {
      block.text = notice + "\n";
    }
    block.fontColor = Colors::DarkBlue;
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
