#pragma once

#include "game/SpecialEventPresenter.h"
#include "ui/pages/PageModalEvent.h"
#include "ui/pages/PageTalkChoice.h"

namespace ui {

class SpecialEventViewMapping {
public:
  static PageTalkChoiceProps toTalkProps(const game::SpecialEventView& view,
                                         int width,
                                         int height);
  static PageModalEventProps toModalProps(const game::SpecialEventView& view,
                                          int width,
                                          int height);

private:
  static PageTalkChoiceItem mapChoice(const game::SpecialEventChoiceView& choice);
  static TextBlock mapEntry(const game::SpecialEventTranscriptEntry& entry,
                            bool historySeparator,
                            bool leadingBlankLine);
  static bmin::String noticeText(const game::SpecialEventTranscriptEntry& entry);
  static bool isNotice(game::SpecialEventTranscriptKind kind);
};

} // namespace ui
