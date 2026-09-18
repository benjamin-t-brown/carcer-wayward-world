#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "in3/SpecialEventRunner.h"

namespace db {
class Database;
}

namespace game {

enum class SpecialEventTranscriptKind {
  Dialogue,
  PlayerChoice,
  JournalNotice,
  ItemReceived
};

struct SpecialEventTranscriptEntry {
  SpecialEventTranscriptKind kind = SpecialEventTranscriptKind::Dialogue;
  bmin::String text;
};

struct SpecialEventChoiceView {
  bmin::String text;
  bmin::String prefix;
  bool previouslyChosen = false;
};

struct SpecialEventView {
  bmin::String title;
  bmin::String portraitSpriteName;
  float portraitScale = 1.5f;
  bmin::DynArray<SpecialEventTranscriptEntry> history;
  bmin::DynArray<SpecialEventTranscriptEntry> current;
  int pinFromBlockIndex = 0;
  bmin::DynArray<SpecialEventChoiceView> choices;
  bool showContinue = false;
  bool finished = false;
  bool isTalk = false;
};

class SpecialEventPresenter {
public:
  static constexpr float kPortraitScale = 1.5f;

  static SpecialEventView view(const in3::SpecialEventRunner& runner,
                               const bmin::DynArray<SpecialEventTranscriptEntry>& history,
                               const db::Database* database,
                               in3::SpecialEventRunnerInterfaceState state);

  static SpecialEventView view(const in3::SpecialEventRunner& runner,
                               const bmin::DynArray<SpecialEventTranscriptEntry>& history,
                               const db::Database* database,
                               in3::SpecialEventRunnerInterface& runnerInterface);

  static void commitTalkCurrent(in3::SpecialEventRunner& runner,
                                bmin::DynArray<SpecialEventTranscriptEntry>& history,
                                const db::Database* database);

  static void commitTalkChoice(in3::SpecialEventRunner& runner,
                               bmin::DynArray<SpecialEventTranscriptEntry>& history,
                               int choiceIndex,
                               const db::Database* database);

  static void consumeModalNotices(in3::SpecialEventRunner& runner);

private:
  static bmin::String resolveItemLabel(const bmin::String& itemName,
                                       const db::Database* database);
  static bmin::String playerChoiceLabel(const in3::DisplayTextChoice& choice);
  static bool hasTranscriptNoticeFrom(
      const bmin::DynArray<SpecialEventTranscriptEntry>& entries,
      int fromIndex,
      SpecialEventTranscriptKind kind,
      const bmin::String& text);
  static void appendNoticeIfNew(bmin::DynArray<SpecialEventTranscriptEntry>& entries,
                                int fromIndex,
                                SpecialEventTranscriptKind kind,
                                const bmin::String& text);
  static void appendCurrentDialogue(bmin::DynArray<SpecialEventTranscriptEntry>& entries,
                                    const in3::SpecialEventRunner& runner);
  static void appendPendingNotices(bmin::DynArray<SpecialEventTranscriptEntry>& entries,
                                   int fromIndex,
                                   bool journalUpdated,
                                   const bmin::DynArray<bmin::String>& receivedItemNames,
                                   const db::Database* database);
};

} // namespace game
