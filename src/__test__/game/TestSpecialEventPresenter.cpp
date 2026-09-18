#include "db/Database.h"
#include "game/SpecialEventPresenter.h"
#include "in3/EventRunnerHelpers.h"
#include "in3/SpecialEventRunner.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/Items.h"
#include "model/templates/SpecialEvents.hpp"
#include "sdl2w/Logger.h"
#include "bmin/DynArray.h"
#include "bmin/Map.h"
#include "bmin/String.h"

namespace {

bool assertTrue(bool condition, const char* label) {
  if (!condition) {
    LOG(ERROR) << label << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertEqualStr(const bmin::String& actual,
                    const bmin::String& expected,
                    const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected '" << expected << "' but got '" << actual << "'"
               << LOG_ENDL;
    return false;
  }
  return true;
}

model::CharacterTemplate makeTalker(const char* name,
                                    const char* talkName,
                                    const char* portraitName) {
  auto character = model::CharacterTemplate{};
  character.name = name;
  character.talk.talkName = talkName;
  character.talk.portraitName = portraitName;
  return character;
}

model::ItemTemplate makeItem(const char* name, const char* label) {
  auto item = model::ItemTemplate{};
  item.name = name;
  item.label = label;
  return item;
}

model::GameEvent makeTalkContinueEvent() {
  auto talkEvent = model::GameEvent{};
  talkEvent.id = "npc_talk";
  talkEvent.title = "Claire";
  talkEvent.eventType = model::GameEventType::TALK;
  talkEvent.icon = "event_icon";

  auto execNode = model::GameEventChildExec{};
  execNode.eventChildType = model::GameEventChildType::EXEC;
  execNode.id = "root";
  execNode.paragraphs = {"Hello there."};
  execNode.next = "end_node";
  execNode.autoAdvance = false;
  talkEvent.children.pushBack(execNode);

  auto endNode = model::GameEventChildEnd{};
  endNode.eventChildType = model::GameEventChildType::END;
  endNode.id = "end_node";
  talkEvent.children.pushBack(endNode);
  return talkEvent;
}

model::GameEvent makeModalContinueEvent() {
  auto modalEvent = model::GameEvent{};
  modalEvent.id = "modal_event";
  modalEvent.eventType = model::GameEventType::MODAL;
  modalEvent.icon = "modal_icon";

  auto execNode = model::GameEventChildExec{};
  execNode.eventChildType = model::GameEventChildType::EXEC;
  execNode.id = "root";
  execNode.paragraphs = {"Body text."};
  execNode.next = "end_node";
  execNode.autoAdvance = false;
  modalEvent.children.pushBack(execNode);

  auto endNode = model::GameEventChildEnd{};
  endNode.eventChildType = model::GameEventChildType::END;
  endNode.id = "end_node";
  modalEvent.children.pushBack(endNode);
  return modalEvent;
}

model::GameEvent makeModalChoiceEvent() {
  auto modalEvent = model::GameEvent{};
  modalEvent.id = "modal_choice";
  modalEvent.title = "A Choice";
  modalEvent.eventType = model::GameEventType::MODAL;
  modalEvent.icon = "modal_icon";

  auto choiceNode = model::GameEventChildChoice{};
  choiceNode.eventChildType = model::GameEventChildType::CHOICE;
  choiceNode.id = "root";
  choiceNode.text = "Pick one:";

  auto choice1 = model::Choice{};
  choice1.text = "First option";
  choice1.prefixText = "A.";
  choice1.next = "end_node";
  choiceNode.choices.pushBack(choice1);

  auto choice2 = model::Choice{};
  choice2.text = "Second option";
  choice2.next = "end_node";
  choiceNode.choices.pushBack(choice2);

  modalEvent.children.pushBack(choiceNode);

  auto endNode = model::GameEventChildEnd{};
  endNode.eventChildType = model::GameEventChildType::END;
  endNode.id = "end_node";
  modalEvent.children.pushBack(endNode);
  return modalEvent;
}

int countKind(const bmin::DynArray<game::SpecialEventTranscriptEntry>& entries,
              game::SpecialEventTranscriptKind kind) {
  auto n = int{0};
  for (int i = 0; i < static_cast<int>(entries.size()); i++) {
    if (entries[i].kind == kind) {
      n++;
    }
  }
  return n;
}

const game::SpecialEventTranscriptEntry*
findKind(const bmin::DynArray<game::SpecialEventTranscriptEntry>& entries,
         game::SpecialEventTranscriptKind kind) {
  for (int i = 0; i < static_cast<int>(entries.size()); i++) {
    if (entries[i].kind == kind) {
      return &entries[i];
    }
  }
  return nullptr;
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestSpecialEventPresenter" << LOG_ENDL;

  auto ok = true;
  db::Database database;
  database.addCharacterTemplate(makeTalker("claire", "npc_talk", "portrait_claire"));
  database.addCharacterTemplate(makeTalker("barto", "other_talk", "portrait_barto"));
  database.addItemTemplate(makeItem("BeerPappysLager", "Pappy's Lager"));

  auto history = bmin::DynArray<game::SpecialEventTranscriptEntry>{};

  {
    auto talkEvent = makeTalkContinueEvent();
    in3::SpecialEventRunner runner({}, talkEvent, {});
    in3::SpecialEventRunnerInterface iface(runner);
    auto before = game::SpecialEventPresenter::view(
        runner, history, &database, iface);
    ok = assertTrue(!before.finished, "before start is not finished") && ok;
    ok = assertTrue(!before.showContinue, "talk does not show modal continue") && ok;
    ok = assertEqualStr(before.title, "Claire", "talk title uses gameEvent.title") &&
         ok;

    iface.startEvent();
    auto view = game::SpecialEventPresenter::view(runner, history, &database, iface);
    ok = assertTrue(view.isTalk, "talk event sets isTalk") && ok;
    ok = assertEqualStr(view.portraitSpriteName,
                        "portrait_claire",
                        "talk portrait uses linked character") &&
         ok;
    ok = assertTrue(view.portraitScale == game::SpecialEventPresenter::kPortraitScale,
                    "portrait scale is presenter constant") &&
         ok;
    ok = assertTrue(view.current.size() == 1 &&
                        view.current[0].kind ==
                            game::SpecialEventTranscriptKind::Dialogue &&
                        view.current[0].text == "Hello there.",
                    "current dialogue is raw display paragraph") &&
         ok;
    ok = assertTrue(view.pinFromBlockIndex == 0, "empty history pins at 0") && ok;
    ok = assertTrue(view.choices.size() == 1 && view.choices[0].isContinue,
                    "synthetic continue choice is flagged isContinue") &&
         ok;
    ok = assertTrue(!view.showContinue, "talk wait-to-select does not showContinue") &&
         ok;
    ok = assertTrue(!view.finished, "talk continue stop is not finished") && ok;
  }

  {
    auto talkEvent = makeTalkContinueEvent();
    talkEvent.title = "";
    in3::SpecialEventRunner runner({}, talkEvent, {});
    in3::SpecialEventRunnerInterface iface(runner);
    iface.startEvent();
    auto view = game::SpecialEventPresenter::view(runner, history, &database, iface);
    ok = assertEqualStr(view.title, "npc_talk", "empty title falls back to id") && ok;
  }

  {
    auto talkEvent = makeTalkContinueEvent();
    in3::SpecialEventRunner runner({}, talkEvent, {});
    in3::setStorage(runner.storage, in3::kTalkPortStorageKey, "barto");
    in3::SpecialEventRunnerInterface iface(runner);
    iface.startEvent();
    auto view = game::SpecialEventPresenter::view(runner, history, &database, iface);
    ok = assertEqualStr(
        view.portraitSpriteName, "portrait_barto", "SET_PORT aux replaces talk owner") &&
         ok;
  }

  {
    auto modalEvent = makeModalContinueEvent();
    in3::SpecialEventRunner runner({}, modalEvent, {});
    in3::SpecialEventRunnerInterface iface(runner);
    iface.startEvent();
    auto view = game::SpecialEventPresenter::view(runner, history, &database, iface);
    ok = assertTrue(!view.isTalk, "modal is not talk") && ok;
    ok = assertTrue(view.showContinue, "modal WAITING_TO_CONTINUE shows continue") &&
         ok;
    ok = assertTrue(!view.finished, "modal with next is not finished") && ok;
    ok = assertEqualStr(view.title, "modal_event", "modal empty title uses id") && ok;
    ok = assertEqualStr(view.portraitSpriteName, "modal_icon", "modal uses event icon") &&
         ok;
    ok = assertTrue(view.choices.empty(), "modal continue stop has no choices") && ok;

    iface.continueEvent();
    auto finishedView =
        game::SpecialEventPresenter::view(runner, history, &database, iface);
    ok = assertTrue(finishedView.finished && iface.isFinished(),
                    "modal empty-next is finished") &&
         ok;
    ok = assertTrue(!finishedView.showContinue, "finished modal does not showContinue") &&
         ok;
  }

  {
    auto modalEvent = makeModalChoiceEvent();
    in3::SpecialEventRunner runner({}, modalEvent, {});
    in3::SpecialEventRunnerInterface iface(runner);
    iface.startEvent();
    auto view = game::SpecialEventPresenter::view(runner, history, &database, iface);
    ok = assertEqualStr(view.title, "A Choice", "choice event uses title") && ok;
    ok = assertTrue(view.choices.size() == 2, "two authored choices") && ok;
    ok = assertEqualStr(view.choices[0].text, "First option", "choice 0 text") && ok;
    ok = assertEqualStr(view.choices[0].prefix, "A.", "choice 0 prefix") && ok;
    ok = assertTrue(!view.choices[0].previouslyChosen && !view.choices[0].isContinue,
                    "choice 0 is unchosen and not continue") &&
         ok;
    ok = assertEqualStr(view.choices[1].text, "Second option", "choice 1 text") && ok;
    ok = assertTrue(view.choices[1].prefix.empty(), "choice 1 has empty prefix") && ok;

    runner.markChoiceChosen(runner.displayTextChoices[0].choiceKey);
    auto chosenView =
        game::SpecialEventPresenter::view(runner, history, &database, iface);
    ok = assertTrue(chosenView.choices[0].previouslyChosen,
                    "wasChoiceChosen marks previouslyChosen") &&
         ok;
    ok = assertTrue(!chosenView.choices[1].previouslyChosen,
                    "unchosen choice stays unmarked") &&
         ok;
  }

  {
    auto talkEvent = makeTalkContinueEvent();
    in3::SpecialEventRunner runner({}, talkEvent, {});
    in3::SpecialEventRunnerInterface iface(runner);
    iface.startEvent();
    runner.pendingJournalNotice = true;
    runner.pendingReceivedItemNames.pushBack("BeerPappysLager");
    runner.pendingReceivedItemNames.pushBack("BeerPappysLager");
    runner.pendingReceivedItemNames.pushBack("UnknownRelic");

    auto view = game::SpecialEventPresenter::view(runner, history, &database, iface);
    ok = assertTrue(runner.pendingJournalNotice &&
                        runner.pendingReceivedItemNames.size() == 3,
                    "view does not consume pending notices") &&
         ok;
    ok = assertTrue(countKind(view.current, game::SpecialEventTranscriptKind::Dialogue) ==
                        1,
                    "current keeps dialogue") &&
         ok;
    ok = assertTrue(
        countKind(view.current, game::SpecialEventTranscriptKind::ItemReceived) == 2,
        "item notices de-dupe identical labels in the current slice") &&
         ok;
    const auto* lager =
        findKind(view.current, game::SpecialEventTranscriptKind::ItemReceived);
    ok = assertTrue(lager != nullptr && lager->text == "Pappy's Lager",
                    "item notice uses template label") &&
         ok;
    auto foundUnknown = false;
    for (int i = 0; i < static_cast<int>(view.current.size()); i++) {
      if (view.current[i].kind == game::SpecialEventTranscriptKind::ItemReceived &&
          view.current[i].text == "UnknownRelic") {
        foundUnknown = true;
      }
    }
    ok = assertTrue(foundUnknown, "missing template falls back to item name") && ok;
    ok = assertTrue(
        countKind(view.current, game::SpecialEventTranscriptKind::JournalNotice) == 1,
        "journal notice is a JournalNotice kind") &&
         ok;
    const auto* journal =
        findKind(view.current, game::SpecialEventTranscriptKind::JournalNotice);
    ok = assertTrue(journal != nullptr && journal->text.empty(),
                    "journal notice text is unused") &&
         ok;
  }

  {
    auto talkHistory = bmin::DynArray<game::SpecialEventTranscriptEntry>{};
    auto talkEvent = makeTalkContinueEvent();
    in3::SpecialEventRunner runner({}, talkEvent, {});
    in3::SpecialEventRunnerInterface iface(runner);
    iface.startEvent();
    runner.pendingJournalNotice = true;
    runner.pendingReceivedItemNames.pushBack("BeerPappysLager");

    game::SpecialEventPresenter::commitTalkChoice(runner, talkHistory, 0, &database);
    ok = assertTrue(!runner.pendingJournalNotice &&
                        runner.pendingReceivedItemNames.empty(),
                    "talk commit consumes pending notices") &&
         ok;
    ok = assertTrue(talkHistory.size() == 4, "dialogue, item, journal, player choice") &&
         ok;
    ok = assertTrue(talkHistory[0].kind == game::SpecialEventTranscriptKind::Dialogue &&
                        talkHistory[0].text == "Hello there.",
                    "talk commit stores raw dialogue") &&
         ok;
    ok = assertTrue(talkHistory[1].kind == game::SpecialEventTranscriptKind::ItemReceived &&
                        talkHistory[1].text == "Pappy's Lager",
                    "talk commit stores resolved item label") &&
         ok;
    ok = assertTrue(talkHistory[2].kind ==
                        game::SpecialEventTranscriptKind::JournalNotice,
                    "talk commit stores journal kind") &&
         ok;
    ok = assertTrue(talkHistory[3].kind == game::SpecialEventTranscriptKind::PlayerChoice &&
                        talkHistory[3].text == "(Continue.)",
                    "talk choice commit stores prefix+text without > ") &&
         ok;

    auto after = game::SpecialEventPresenter::view(
        runner, talkHistory, &database, iface);
    ok = assertTrue(after.pinFromBlockIndex == static_cast<int>(talkHistory.size()),
                    "pinFromBlockIndex is history size before current") &&
         ok;
    ok = assertTrue(countKind(after.current,
                              game::SpecialEventTranscriptKind::ItemReceived) == 0 &&
                        countKind(after.current,
                                  game::SpecialEventTranscriptKind::JournalNotice) == 0,
                    "consumed notices are not re-shown in current") &&
         ok;
  }

  {
    auto talkHistory = bmin::DynArray<game::SpecialEventTranscriptEntry>{};
    auto talkEvent = makeTalkContinueEvent();
    in3::SpecialEventRunner runner({}, talkEvent, {});
    in3::SpecialEventRunnerInterface iface(runner);
    iface.startEvent();
    runner.pendingJournalNotice = true;
    game::SpecialEventPresenter::commitTalkCurrent(runner, talkHistory, &database);
    ok = assertTrue(talkHistory.size() == 2 &&
                        talkHistory[1].kind ==
                            game::SpecialEventTranscriptKind::JournalNotice,
                    "talk current commit appends dialogue and journal") &&
         ok;
    ok = assertTrue(
        countKind(talkHistory, game::SpecialEventTranscriptKind::PlayerChoice) == 0,
        "current commit does not append a player choice") &&
         ok;
  }

  {
    auto modalHistory = bmin::DynArray<game::SpecialEventTranscriptEntry>{};
    auto modalEvent = makeModalContinueEvent();
    in3::SpecialEventRunner runner({}, modalEvent, {});
    in3::SpecialEventRunnerInterface iface(runner);
    iface.startEvent();
    runner.pendingJournalNotice = true;
    runner.pendingReceivedItemNames.pushBack("BeerPappysLager");
    const auto historySizeBefore = modalHistory.size();
    game::SpecialEventPresenter::consumeModalNotices(runner);
    ok = assertTrue(!runner.pendingJournalNotice &&
                        runner.pendingReceivedItemNames.empty(),
                    "modal consume clears pending notices") &&
         ok;
    ok = assertTrue(modalHistory.size() == historySizeBefore,
                    "modal consume does not append history") &&
         ok;
  }

  {
    auto talkEvent = makeTalkContinueEvent();
    in3::SpecialEventRunner runner({}, talkEvent, {});
    in3::SpecialEventRunnerInterface iface(runner);
    iface.startEvent();
    auto choice = in3::DisplayTextChoice{};
    choice.text = "Keep walking";
    choice.prefix = "1.";
    choice.choiceKey = "root:0";
    runner.displayTextChoices.pushBack(choice);
    auto view = game::SpecialEventPresenter::view(runner, history, &database, iface);
    auto talkHistory = bmin::DynArray<game::SpecialEventTranscriptEntry>{};
    game::SpecialEventPresenter::commitTalkChoice(
        runner, talkHistory, static_cast<int>(runner.displayTextChoices.size()) - 1,
        &database);
    ok = assertTrue(!talkHistory.empty() &&
                        talkHistory.back().kind ==
                            game::SpecialEventTranscriptKind::PlayerChoice &&
                        talkHistory.back().text == "1. Keep walking",
                    "player choice label is prefix plus text") &&
         ok;
    ok = assertTrue(view.choices.size() >= 2 && !view.choices.back().isContinue,
                    "authored choice is not isContinue") &&
         ok;
  }

  if (!ok) {
    LOG(ERROR) << "TestSpecialEventPresenter failed" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestSpecialEventPresenter completed successfully" << LOG_ENDL;
  return 0;
}
