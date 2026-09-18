#include "game/SpecialEventPresenter.h"
#include "db/Database.h"
#include "game/TalkEventPortrait.h"
#include "in3/EventRunnerHelpers.h"

#include <string_view>

namespace game {

bmin::String SpecialEventPresenter::resolveItemLabel(const bmin::String& itemName,
                                                     const db::Database* database) {
  if (!database) {
    return itemName;
  }
  const auto* item = database->findItemTemplate(
      std::string_view(itemName.cStr(), itemName.size()));
  if (item && !item->label.empty()) {
    return item->label;
  }
  return itemName;
}

bmin::String SpecialEventPresenter::playerChoiceLabel(const in3::DisplayTextChoice& choice) {
  if (choice.prefix.empty()) {
    return choice.text;
  }
  return choice.prefix + " " + choice.text;
}

bool SpecialEventPresenter::hasTranscriptNoticeFrom(
    const bmin::DynArray<SpecialEventTranscriptEntry>& entries,
    int fromIndex,
    SpecialEventTranscriptKind kind,
    const bmin::String& text) {
  for (int i = fromIndex; i < static_cast<int>(entries.size()); i++) {
    const auto& entry = entries[i];
    if (entry.kind != kind) {
      continue;
    }
    if (kind == SpecialEventTranscriptKind::JournalNotice || entry.text == text) {
      return true;
    }
  }
  return false;
}

void SpecialEventPresenter::appendNoticeIfNew(
    bmin::DynArray<SpecialEventTranscriptEntry>& entries,
    int fromIndex,
    SpecialEventTranscriptKind kind,
    const bmin::String& text) {
  if (kind == SpecialEventTranscriptKind::ItemReceived && text.empty()) {
    return;
  }
  if (hasTranscriptNoticeFrom(entries, fromIndex, kind, text)) {
    return;
  }
  auto entry = SpecialEventTranscriptEntry{};
  entry.kind = kind;
  entry.text = text;
  entries.pushBack(entry);
}

void SpecialEventPresenter::appendCurrentDialogue(
    bmin::DynArray<SpecialEventTranscriptEntry>& entries,
    const in3::SpecialEventRunner& runner) {
  if (runner.displayText.empty()) {
    return;
  }
  auto entry = SpecialEventTranscriptEntry{};
  entry.kind = SpecialEventTranscriptKind::Dialogue;
  entry.text = runner.displayText;
  entries.pushBack(entry);
}

void SpecialEventPresenter::appendPendingNotices(
    bmin::DynArray<SpecialEventTranscriptEntry>& entries,
    int fromIndex,
    bool journalUpdated,
    const bmin::DynArray<bmin::String>& receivedItemNames,
    const db::Database* database) {
  for (const auto& itemName : receivedItemNames) {
    appendNoticeIfNew(entries,
                      fromIndex,
                      SpecialEventTranscriptKind::ItemReceived,
                      resolveItemLabel(itemName, database));
  }
  if (journalUpdated) {
    appendNoticeIfNew(
        entries, fromIndex, SpecialEventTranscriptKind::JournalNotice, {});
  }
}

SpecialEventView SpecialEventPresenter::view(
    const in3::SpecialEventRunner& runner,
    const bmin::DynArray<SpecialEventTranscriptEntry>& history,
    const db::Database* database,
    in3::SpecialEventRunnerInterfaceState state) {
  auto result = SpecialEventView{};
  result.title =
      runner.gameEvent.title.empty() ? runner.gameEvent.id : runner.gameEvent.title;
  result.isTalk = runner.gameEvent.eventType == model::GameEventType::TALK;
  result.finished = state == in3::SpecialEventRunnerInterfaceState::FINISHED;
  result.showContinue = !result.isTalk &&
                        state == in3::SpecialEventRunnerInterfaceState::WAITING_TO_CONTINUE;
  result.portraitScale = kPortraitScale;
  result.history = history;
  result.pinFromBlockIndex = static_cast<int>(history.size());

  const auto auxName =
      in3::getStorage(runner.storage, in3::kTalkPortStorageKey).value_or("");
  result.portraitSpriteName =
      resolveTalkEventPortrait(runner.gameEvent, database, auxName);

  appendCurrentDialogue(result.current, runner);
  appendPendingNotices(result.current,
                       0,
                       runner.pendingJournalNotice,
                       runner.pendingReceivedItemNames,
                       database);

  for (const auto& choice : runner.displayTextChoices) {
    auto choiceView = SpecialEventChoiceView{};
    choiceView.text = choice.text;
    choiceView.prefix = choice.prefix;
    choiceView.previouslyChosen = runner.wasChoiceChosen(choice.choiceKey);
    choiceView.isContinue = in3::isContinueChoice(choice);
    result.choices.pushBack(choiceView);
  }
  return result;
}

SpecialEventView SpecialEventPresenter::view(
    const in3::SpecialEventRunner& runner,
    const bmin::DynArray<SpecialEventTranscriptEntry>& history,
    const db::Database* database,
    in3::SpecialEventRunnerInterface& runnerInterface) {
  return view(runner, history, database, runnerInterface.getState());
}

void SpecialEventPresenter::commitTalkCurrent(
    in3::SpecialEventRunner& runner,
    bmin::DynArray<SpecialEventTranscriptEntry>& history,
    const db::Database* database) {
  const auto fromIndex = static_cast<int>(history.size());
  appendCurrentDialogue(history, runner);
  const auto notices = runner.consumePendingNotices();
  appendPendingNotices(history,
                       fromIndex,
                       notices.journalUpdated,
                       notices.receivedItemNames,
                       database);
}

void SpecialEventPresenter::commitTalkChoice(
    in3::SpecialEventRunner& runner,
    bmin::DynArray<SpecialEventTranscriptEntry>& history,
    int choiceIndex,
    const db::Database* database) {
  commitTalkCurrent(runner, history, database);
  if (choiceIndex < 0 ||
      static_cast<size_t>(choiceIndex) >= runner.displayTextChoices.size()) {
    return;
  }
  auto entry = SpecialEventTranscriptEntry{};
  entry.kind = SpecialEventTranscriptKind::PlayerChoice;
  entry.text = playerChoiceLabel(runner.displayTextChoices[choiceIndex]);
  history.pushBack(entry);
}

void SpecialEventPresenter::consumeModalNotices(in3::SpecialEventRunner& runner) {
  runner.consumePendingNotices();
}

} // namespace game
