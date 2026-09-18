#include "LayerSpecialEvent.h"
#include "actions/navigation/UiContinueSpecialEvent.hpp"
#include "actions/navigation/UiRemoveLayer.hpp"
#include "actions/navigation/UiSelectSpecialEventChoice.hpp"
#include "db/Database.h"
#include "game/TalkEventPortrait.h"
#include "in3/EventRunnerHelpers.h"
#include "sdl2w/L10n.h"
#include "state/State.hpp"
#include "state/StateManager.h"
#include "ui/colors.hpp"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonGroup.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/elements/buttons/ButtonTextWrap.h"
#include "ui/observers/ActionObserver.hpp"
#include "ui/pages/PageModalEvent.h"
#include "ui/pages/PageTalkChoice.h"
#include <bmin/StringInterop.h>
#include <string_view>

namespace layers {

namespace {

constexpr int TALK_CHOICE_AREA_HEIGHT = 250;
constexpr int kKeyboardPressFlashMs = 120;

bmin::String journalNoticeText() { return TRANSLATE("Your journal has been updated."); }

bmin::String itemReceivedNoticeText(const bmin::String& itemName,
                                    const db::Database* database) {
  bmin::String label = itemName;
  if (database) {
    const auto* item = database->findItemTemplate(
        std::string_view(itemName.cStr(), itemName.size()));
    if (item && !item->label.empty()) {
      label = item->label;
    }
  }
  return TRANSLATE("You have received ") + label + ".";
}

bool blockIsGreyNotice(const ui::TextBlock& block, const bmin::String& notice) {
  if (!block.fontColor.has_value()) {
    return false;
  }
  return block.text == notice || block.text == notice + "\n\n" ||
         block.text == bmin::String("\n\n") + notice;
}

bool hasGreyNoticeFrom(const bmin::DynArray<ui::TextBlock>& blocks,
                       int fromIndex,
                       const bmin::String& notice) {
  for (int i = fromIndex; i < static_cast<int>(blocks.size()); i++) {
    if (blockIsGreyNotice(blocks[i], notice)) {
      return true;
    }
  }
  return false;
}

void appendGreyNotice(bmin::DynArray<ui::TextBlock>& blocks,
                      int fromIndex,
                      const bmin::String& notice,
                      bool leadingBlankLine) {
  if (notice.empty() || hasGreyNoticeFrom(blocks, fromIndex, notice)) {
    return;
  }
  ui::TextBlock block;
  block.text = leadingBlankLine ? bmin::String("\n\n") + notice : notice;
  block.fontColor = ui::Colors::Grey;
  blocks.pushBack(block);
}

void appendPendingSystemNotices(bmin::DynArray<ui::TextBlock>& blocks,
                                in3::SpecialEventRunner& runner,
                                int fromIndex,
                                bool leadingBlankLine,
                                const db::Database* database) {
  bool usedLeadingBlank = false;
  for (const auto& itemName : runner.pendingReceivedItemNames) {
    const bmin::String notice = itemReceivedNoticeText(itemName, database);
    appendGreyNotice(blocks, fromIndex, notice, leadingBlankLine && !usedLeadingBlank);
    usedLeadingBlank = usedLeadingBlank || leadingBlankLine;
  }
  if (runner.pendingJournalNotice) {
    appendGreyNotice(blocks,
                     fromIndex,
                     journalNoticeText(),
                     leadingBlankLine && !usedLeadingBlank);
  }
}

ui::PageTalkChoiceProps buildTalkProps(in3::SpecialEventRunner& runner,
                                       const bmin::DynArray<ui::TextBlock>& talkHistory,
                                       int windowWidth,
                                       int windowHeight,
                                       const db::Database* database) {
  ui::PageTalkChoiceProps props;
  props.width = windowWidth;
  props.height = windowHeight;
  props.choiceAreaHeight = TALK_CHOICE_AREA_HEIGHT;
  props.title =
      runner.gameEvent.title.empty() ? runner.gameEvent.id : runner.gameEvent.title;
  const auto auxName =
      in3::getStorage(runner.storage, in3::kTalkPortStorageKey).value_or("");
  props.portraitSpriteName =
      game::resolveTalkEventPortrait(runner.gameEvent, database, auxName);
  props.portraitScale = 1.5f;
  props.pinFromBlockIndex = static_cast<int>(talkHistory.size());
  for (const auto& block : talkHistory) {
    props.textBlocks.pushBack(block);
  }
  if (!runner.displayText.empty()) {
    ui::TextBlock block;
    block.text = runner.displayText;
    props.textBlocks.pushBack(block);
  }
  appendPendingSystemNotices(
      props.textBlocks, runner, props.pinFromBlockIndex, false, database);
  for (const auto& choice : runner.displayTextChoices) {
    ui::PageTalkChoiceItem item;
    item.nextId = choice.next;
    item.text = choice.text;
    item.prefixText = choice.prefix;
    item.previouslyChosen = runner.wasChoiceChosen(choice.choiceKey);
    props.choices.pushBack(item);
  }
  return props;
}

ui::PageModalEventProps buildModalProps(in3::SpecialEventRunner& runner,
                                        int windowWidth,
                                        int windowHeight,
                                        const db::Database* database) {
  ui::PageModalEventProps props;
  // Window dims; ModalSmall default CappedCentered sizes/centers the shell.
  props.width = windowWidth;
  props.height = windowHeight;
  props.title =
      runner.gameEvent.title.empty() ? runner.gameEvent.id : runner.gameEvent.title;
  if (!runner.displayText.empty()) {
    ui::TextBlock block;
    block.text = runner.displayText;
    props.textBlocks.pushBack(block);
  }
  appendPendingSystemNotices(
      props.textBlocks, runner, 0, !runner.displayText.empty(), database);
  for (const auto& choice : runner.displayTextChoices) {
    ui::PageTalkChoiceItem item;
    item.nextId = choice.next;
    item.text = choice.text;
    item.prefixText = choice.prefix;
    item.previouslyChosen = runner.wasChoiceChosen(choice.choiceKey);
    props.choices.pushBack(item);
  }
  props.showContinueButton = props.choices.empty() && !runner.displayText.empty() &&
                             !runner.getNextNodeId().empty();
  return props;
}

} // namespace

LayerSpecialEvent::LayerSpecialEvent(
    sdl2w::Window* _window,
    const model::GameEvent& gameEvent,
    const bmin::Map<bmin::String, model::GameEvent>& gameEvents,
    const bmin::Map<bmin::String, bmin::String>& initialStorage)
    : UiLayer(_window, LAYER_ID),
      runner(initialStorage, gameEvent, gameEvents),
      runnerInterface(runner) {

  if (!assertInterfaces()) {
    remove();
    return;
  }

  subscribeAction<state::ActionEvent::UiContinueSpecialEvent>(
      [this](auto&, auto&) { onContinue(); });
  subscribeAction<state::ActionEvent::UiSelectSpecialEventChoice>(
      [this](state::AbstractAction& action, auto&) {
        onChoiceSelected(action.getEventValue());
      });

  auto [windowWidth, windowHeight] = window->getDims();
  const auto scale = 1.f;

  if (gameEvent.eventType == model::GameEventType::TALK) {
    auto pageTalkChoice = new ui::PageTalkChoice(window);
    pageTalkChoice->setId("eventPage");
    pageTalkChoice->setPos(0, 0);
    pageTalkChoice->setScale(scale);
    pageTalkChoice->setProps(buildTalkProps(runner,
                                            talkHistory,
                                            static_cast<int>(windowWidth / scale),
                                            static_cast<int>(windowHeight / scale),
                                            getDatabase()));
    addUiElement(bmin::UniquePtr<ui::UiElement>(pageTalkChoice));
  } else {
    auto modalProps = buildModalProps(runner,
                                      static_cast<int>(windowWidth / scale),
                                      static_cast<int>(windowHeight / scale),
                                      getDatabase());
    auto pageModalEvent = new ui::PageModalEvent(window);
    pageModalEvent->setId("eventPage");
    pageModalEvent->setPos(0, 0);
    pageModalEvent->setScale(scale);
    pageModalEvent->setProps(modalProps);
    addUiElement(bmin::UniquePtr<ui::UiElement>(pageModalEvent));
  }

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(bmin::UniquePtr<ui::UiElement>(floatingNotificationSection));

  runnerInterface.startEvent();
  syncUi();
  setupTalkKeyboardScroll();
}

void LayerSpecialEvent::appendCurrentTalkTextToHistory() {
  if (!runner.displayText.empty()) {
    ui::TextBlock block;
    block.text = runner.displayText + "\n\n";
    talkHistory.pushBack(block);
  }
  const auto* database = getDatabase();
  for (const auto& itemName : runner.pendingReceivedItemNames) {
    const bmin::String notice = itemReceivedNoticeText(itemName, database);
    if (talkHistory.empty() || !blockIsGreyNotice(talkHistory.back(), notice)) {
      ui::TextBlock block;
      block.text = notice + bmin::String("\n\n");
      block.fontColor = ui::Colors::Grey;
      talkHistory.pushBack(block);
    }
  }
  runner.pendingReceivedItemNames.clear();
  if (runner.pendingJournalNotice) {
    const bmin::String notice = journalNoticeText();
    if (talkHistory.empty() || !blockIsGreyNotice(talkHistory.back(), notice)) {
      ui::TextBlock block;
      block.text = notice + bmin::String("\n\n");
      block.fontColor = ui::Colors::Grey;
      talkHistory.pushBack(block);
    }
    runner.pendingJournalNotice = false;
  }
}

void LayerSpecialEvent::appendTalkChoiceToHistory(int choiceIndex) {
  appendCurrentTalkTextToHistory();
  if (choiceIndex < 0 ||
      static_cast<size_t>(choiceIndex) >= runner.displayTextChoices.size()) {
    return;
  }
  const auto& choice = runner.displayTextChoices[choiceIndex];
  const bmin::String label =
      choice.prefix.empty() ? choice.text : choice.prefix + " " + choice.text;
  ui::TextBlock block;
  block.text = bmin::String("> ") + label + "\n\n";
  block.fontColor = ui::Colors::DarkBlue;
  talkHistory.pushBack(block);
}

void LayerSpecialEvent::syncUi() {
  if (runner.gameEvent.eventType == model::GameEventType::TALK) {
    auto* pageTalkChoice = getUiElement<ui::PageTalkChoice>("eventPage");
    if (!pageTalkChoice) {
      return;
    }
    auto [windowWidth, windowHeight] = window->getDims();
    pageTalkChoice->setProps(buildTalkProps(runner,
                                            talkHistory,
                                            static_cast<int>(windowWidth),
                                            static_cast<int>(windowHeight),
                                            getDatabase()));
    attachChoiceObservers();
    return;
  }

  auto* pageModalEvent = getUiElement<ui::PageModalEvent>("eventPage");
  if (!pageModalEvent) {
    return;
  }
  auto [windowWidth, windowHeight] = window->getDims();
  auto modalProps = buildModalProps(runner,
                                    static_cast<int>(windowWidth),
                                    static_cast<int>(windowHeight),
                                    getDatabase());
  pageModalEvent->setPos(0, 0);
  pageModalEvent->setProps(modalProps);
  attachChoiceObservers();
  attachModalContinueObserver();
}

void LayerSpecialEvent::attachChoiceObservers() {
  if (runner.gameEvent.eventType == model::GameEventType::TALK) {
    auto* pageTalkChoice = getUiElement<ui::PageTalkChoice>("eventPage");
    if (!pageTalkChoice) {
      return;
    }
    for (int i = 0; i < static_cast<int>(runner.displayTextChoices.size()); i++) {
      const auto choiceId = "choice" + bmin::toString(i);
      auto* choice = pageTalkChoice->getChildById(
          std::string_view(choiceId.cStr(), choiceId.size()));
      if (!choice) {
        continue;
      }
      choice->addEventObserver(
          ui::makeActionObserver<state::actions::UiSelectSpecialEventChoice>(i));
    }
    return;
  }

  auto* pageModalEvent = getUiElement<ui::PageModalEvent>("eventPage");
  if (!pageModalEvent) {
    return;
  }
  for (int i = 0; i < static_cast<int>(runner.displayTextChoices.size()); i++) {
    const auto choiceId = "choice" + bmin::toString(i);
    auto* choice =
        pageModalEvent->getChildById(std::string_view(choiceId.cStr(), choiceId.size()));
    if (!choice) {
      continue;
    }
    choice->addEventObserver(
        ui::makeActionObserver<state::actions::UiSelectSpecialEventChoice>(i));
  }
}

void LayerSpecialEvent::attachModalContinueObserver() {
  if (runner.gameEvent.eventType == model::GameEventType::TALK) {
    return;
  }

  auto* button = findModalContinueButton();
  if (!button) {
    return;
  }
  button->addEventObserver(
      ui::makeActionObserver<state::actions::UiContinueSpecialEvent>());
}

ui::ButtonModal* LayerSpecialEvent::findModalContinueButton() {
  if (runner.gameEvent.eventType == model::GameEventType::TALK) {
    return nullptr;
  }

  auto* pageModalEvent = getUiElement<ui::PageModalEvent>("eventPage");
  if (!pageModalEvent) {
    return nullptr;
  }
  auto* modal = pageModalEvent->getChildById("modal");
  if (!modal) {
    return nullptr;
  }
  auto* buttonGroup = dynamic_cast<ui::ButtonGroup*>(modal->getChildById("buttonGroup"));
  if (!buttonGroup || buttonGroup->getChildren().empty()) {
    return nullptr;
  }
  return dynamic_cast<ui::ButtonModal*>(buttonGroup->getChildren()[0].get());
}

void LayerSpecialEvent::beginKeyboardContinuePress() {
  if (continuePressRemainingMs > 0 || choicePressRemainingMs > 0) {
    return;
  }

  // TALK injects a synthetic "(Continue.)" choice for non-auto-advance EXEC stops.
  if (runner.gameEvent.eventType == model::GameEventType::TALK) {
    if (runner.displayTextChoices.size() == 1 &&
        runner.displayTextChoices[0].text == TRANSLATE("(Continue.)")) {
      beginKeyboardChoicePress(0);
      return;
    }
    return;
  }

  if (!runner.displayTextChoices.empty()) {
    return;
  }

  if (auto* button = findModalContinueButton()) {
    button->isActive = true;
    continuePressRemainingMs = kKeyboardPressFlashMs;
    return;
  }

  onContinue();
}

ui::ButtonTextWrap* LayerSpecialEvent::findChoiceButton(int choiceIndex) {
  const auto choiceId = "choice" + bmin::toString(choiceIndex);
  const auto choiceIdView = std::string_view(choiceId.cStr(), choiceId.size());

  if (runner.gameEvent.eventType == model::GameEventType::TALK) {
    auto* pageTalkChoice = getUiElement<ui::PageTalkChoice>("eventPage");
    if (!pageTalkChoice) {
      return nullptr;
    }
    return dynamic_cast<ui::ButtonTextWrap*>(pageTalkChoice->getChildById(choiceIdView));
  }

  auto* pageModalEvent = getUiElement<ui::PageModalEvent>("eventPage");
  if (!pageModalEvent) {
    return nullptr;
  }
  return dynamic_cast<ui::ButtonTextWrap*>(pageModalEvent->getChildById(choiceIdView));
}

void LayerSpecialEvent::beginKeyboardChoicePress(int choiceIndex) {
  if (continuePressRemainingMs > 0 || choicePressRemainingMs > 0 || eventFinished) {
    return;
  }
  if (choiceIndex < 0 ||
      static_cast<size_t>(choiceIndex) >= runner.displayTextChoices.size()) {
    return;
  }

  pendingChoiceIndex = choiceIndex;
  choicePressRemainingMs = kKeyboardPressFlashMs;
  if (auto* button = findChoiceButton(choiceIndex)) {
    button->isActive = true;
  }
}

void LayerSpecialEvent::onChoiceSelected(int choiceIndex) {
  if (eventFinished) {
    return;
  }
  talkKeyboardScroll.stopScroll();
  if (runner.gameEvent.eventType == model::GameEventType::TALK) {
    appendTalkChoiceToHistory(choiceIndex);
  } else {
    runner.pendingJournalNotice = false;
    runner.pendingReceivedItemNames.clear();
  }
  runnerInterface.selectChoice(choiceIndex);
  if (runner.gameEvent.eventType == model::GameEventType::TALK && runner.isAtEndNode()) {
    eventFinished = true;
    closeLayer();
    return;
  }
  if (runner.getNextNodeId().empty() && runner.displayTextChoices.empty() &&
      !runner.displayText.empty()) {
    eventFinished = true;
    closeLayer();
    return;
  }
  needsSyncUi = true;
}

void LayerSpecialEvent::onContinue() {
  if (eventFinished) {
    closeLayer();
    return;
  }

  if (!runner.displayTextChoices.empty()) {
    return;
  }

  if (runner.getNextNodeId().empty()) {
    eventFinished = true;
    closeLayer();
    return;
  }

  if (runner.gameEvent.eventType == model::GameEventType::TALK) {
    appendCurrentTalkTextToHistory();
  } else {
    runner.pendingJournalNotice = false;
    runner.pendingReceivedItemNames.clear();
  }
  runnerInterface.continueEvent();
  if (runner.gameEvent.eventType == model::GameEventType::TALK && runner.isAtEndNode()) {
    eventFinished = true;
    closeLayer();
    return;
  }
  if (runner.getNextNodeId().empty() && runner.displayTextChoices.empty()) {
    closeLayer();
    return;
  }
  needsSyncUi = true;
}

void LayerSpecialEvent::persistRunnerStorage() {
  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  auto& persisted = stateManager->getState().specialEventStorage;
  persisted = runner.storage;
  in3::clearTmpStorageKeys(persisted);
}

void LayerSpecialEvent::closeLayer() {
  talkKeyboardScroll.stopScroll();
  choicePressRemainingMs = 0;
  pendingChoiceIndex.reset();
  continuePressRemainingMs = 0;
  persistRunnerStorage();
  auto stateManager = getStateManager();
  if (!stateManager) {
    remove();
    return;
  }
  stateManager->enqueueAction(
      state::makeAction<state::actions::UiRemoveLayer>(bmin::fromStringView(LAYER_ID)),
      0);
}

ui::SectionScrollable* LayerSpecialEvent::getTalkTextSection() {
  auto* pageTalkChoice = getUiElement<ui::PageTalkChoice>("eventPage");
  if (!pageTalkChoice) {
    return nullptr;
  }
  return dynamic_cast<ui::SectionScrollable*>(
      pageTalkChoice->getChildById("textSection"));
}

ui::SectionScrollable* LayerSpecialEvent::getTalkChoiceSection() {
  auto* pageTalkChoice = getUiElement<ui::PageTalkChoice>("eventPage");
  if (!pageTalkChoice) {
    return nullptr;
  }
  return dynamic_cast<ui::SectionScrollable*>(
      pageTalkChoice->getChildById("choiceSection"));
}

void LayerSpecialEvent::setupTalkKeyboardScroll() {
  talkKeyboardScroll.clearBindings();
  if (runner.gameEvent.eventType != model::GameEventType::TALK) {
    return;
  }

  // Left/right (and numpad) scroll dialogue history; up/down scroll choices.
  // Section getters resolve live pointers so rebuilds in syncUi stay safe.
  auto textSection = [this]() { return getTalkTextSection(); };
  auto choiceSection = [this]() { return getTalkChoiceSection(); };

  talkKeyboardScroll.bindSectionKey("Left", textSection, ui::ScrollDirection::Up);
  talkKeyboardScroll.bindSectionKey("Keypad 4", textSection, ui::ScrollDirection::Up);
  talkKeyboardScroll.bindSectionKey("Right", textSection, ui::ScrollDirection::Down);
  talkKeyboardScroll.bindSectionKey("Keypad 6", textSection, ui::ScrollDirection::Down);
  talkKeyboardScroll.bindSectionKey("Up", choiceSection, ui::ScrollDirection::Up);
  talkKeyboardScroll.bindSectionKey("Keypad 8", choiceSection, ui::ScrollDirection::Up);
  talkKeyboardScroll.bindSectionKey("Down", choiceSection, ui::ScrollDirection::Down);
  talkKeyboardScroll.bindSectionKey("Keypad 2", choiceSection, ui::ScrollDirection::Down);
}

void LayerSpecialEvent::onKeyDown(std::string_view key, int /*keyCode*/) {
  if (getState() != LayerState::ON) {
    return;
  }

  // Talk must be exited via choices / the modal close control — not Escape.
  if (key == "Escape") {
    if (runner.gameEvent.eventType != model::GameEventType::TALK) {
      closeLayer();
    }
    return;
  }

  if (talkKeyboardScroll.onKeyDown(key)) {
    return;
  }

  if (key == "Return" || key == "Keypad Enter" || key == "space") {
    talkKeyboardScroll.stopScroll();
    beginKeyboardContinuePress();
    return;
  }

  if (key.size() == 1 && key[0] >= '1' && key[0] <= '9') {
    talkKeyboardScroll.stopScroll();
    const auto choiceIndex = static_cast<int>(key[0] - '1');
    beginKeyboardChoicePress(choiceIndex);
  }
}

void LayerSpecialEvent::onKeyUp(std::string_view key, int /*keyCode*/) {
  talkKeyboardScroll.onKeyUp(key);
}

void LayerSpecialEvent::update(int deltaTime) {
  UiLayer::update(deltaTime);
  talkKeyboardScroll.update(deltaTime, window);

  if (choicePressRemainingMs > 0) {
    choicePressRemainingMs -= deltaTime;
    if (choicePressRemainingMs <= 0) {
      choicePressRemainingMs = 0;
      const int choiceIndex = pendingChoiceIndex.value_or(-1);
      pendingChoiceIndex.reset();
      if (auto* button = findChoiceButton(choiceIndex)) {
        button->isActive = false;
      }
      if (choiceIndex >= 0) {
        onChoiceSelected(choiceIndex);
      }
    }
  }

  if (continuePressRemainingMs > 0) {
    continuePressRemainingMs -= deltaTime;
    if (continuePressRemainingMs <= 0) {
      continuePressRemainingMs = 0;
      if (auto* button = findModalContinueButton()) {
        button->isActive = false;
      }
      onContinue();
    }
  }

  if (needsSyncUi) {
    needsSyncUi = false;
    syncUi();
  }
}

} // namespace layers
