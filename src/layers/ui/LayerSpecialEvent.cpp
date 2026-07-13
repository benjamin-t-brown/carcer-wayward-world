#include "LayerSpecialEvent.h"
#include "runner/EventRunnerHelpers.h"
#include "sdl2w/Logger.h"
#include "state/State.h"
#include "state/StateManager.h"
#include "state/actions/ui/UiRemoveLayer.hpp"
#include "ui/colors.h"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/elements/SectionScrollable.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonGroup.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/observers/ObserverRemoveLayer.hpp"
#include "ui/observers/ObserverSpecialEventChoice.hpp"
#include "ui/observers/ObserverSpecialEventContinue.hpp"
#include "ui/pages/PageModalEvent.h"
#include "ui/pages/PageTalkChoice.h"
#include "ui/elements/buttons/ButtonTextWrap.h"
#include <algorithm>
#include <string_view>

namespace layers {

namespace {

constexpr int TALK_CHOICE_AREA_HEIGHT = 250;
constexpr int MODAL_WIDTH = 500;
constexpr int kKeyboardPressFlashMs = 120;

ui::PageTalkChoiceProps buildTalkProps(runner::SpecialEventRunner& runner,
                                       const bmin::DynArray<ui::TextBlock>& talkHistory,
                                       int windowWidth,
                                       int windowHeight) {
  ui::PageTalkChoiceProps props;
  props.width = windowWidth;
  props.height = windowHeight;
  props.choiceAreaHeight = TALK_CHOICE_AREA_HEIGHT;
  props.title =
      runner.gameEvent.title.empty() ? runner.gameEvent.id : runner.gameEvent.title;
  props.portraitSpriteName = runner.gameEvent.icon;
  props.pinFromBlockIndex = static_cast<int>(talkHistory.size());
  for (const auto& block : talkHistory) {
    props.textBlocks.pushBack(block);
  }
  if (!runner.displayText.empty()) {
    ui::TextBlock block;
    block.text = runner.displayText;
    props.textBlocks.pushBack(block);
  }
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

ui::PageModalEventProps
buildModalProps(runner::SpecialEventRunner& runner, int windowWidth, int windowHeight) {
  ui::PageModalEventProps props;
  props.width = MODAL_WIDTH;
  props.height = std::min(windowHeight - 50, 420);
  props.title =
      runner.gameEvent.title.empty() ? runner.gameEvent.id : runner.gameEvent.title;
  if (!runner.displayText.empty()) {
    ui::TextBlock block;
    block.text = runner.displayText;
    props.textBlocks.pushBack(block);
  }
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
    : Layer(_window, LAYER_ID),
      runner(initialStorage, gameEvent, gameEvents),
      runnerInterface(runner) {

  if (!assertInterfaces()) {
    remove();
    return;
  }

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
                                            static_cast<int>(windowHeight / scale)));
    addUiElement(pageTalkChoice);
  } else {
    auto modalProps = buildModalProps(runner,
                                      static_cast<int>(windowWidth / scale),
                                      static_cast<int>(windowHeight / scale));
    auto pageModalEvent = new ui::PageModalEvent(window);
    pageModalEvent->setId("eventPage");
    pageModalEvent->setPos((static_cast<int>(windowWidth / scale) - modalProps.width) / 2,
                           (static_cast<int>(windowHeight / scale) - modalProps.height) /
                               2);
    pageModalEvent->setScale(scale);
    pageModalEvent->setProps(modalProps);
    addUiElement(pageModalEvent);
  }

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);

  runnerInterface.startEvent();
  syncUi();
  setupTalkKeyboardScroll();
}

void LayerSpecialEvent::appendCurrentTalkTextToHistory() {
  if (runner.displayText.empty()) {
    return;
  }
  ui::TextBlock block;
  block.text = runner.displayText + "\n\n";
  talkHistory.pushBack(block);
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
                                            static_cast<int>(windowHeight)));
    attachChoiceObservers();
    return;
  }

  auto* pageModalEvent = getUiElement<ui::PageModalEvent>("eventPage");
  if (!pageModalEvent) {
    return;
  }
  auto [windowWidth, windowHeight] = window->getDims();
  auto modalProps = buildModalProps(
      runner, static_cast<int>(windowWidth), static_cast<int>(windowHeight));
  pageModalEvent->setPos((static_cast<int>(windowWidth) - modalProps.width) / 2,
                         (static_cast<int>(windowHeight) - modalProps.height) / 2);
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
      choice->addEventObserver(new ui::ObserverSpecialEventChoice(this, i));
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
    choice->addEventObserver(new ui::ObserverSpecialEventChoice(this, i));
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
  button->addEventObserver(new ui::ObserverSpecialEventContinue(this));
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
        runner.displayTextChoices[0].text == "(Continue.)") {
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
  talkKeyboardScroll.clearHeld();
  if (runner.gameEvent.eventType == model::GameEventType::TALK) {
    appendTalkChoiceToHistory(choiceIndex);
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

void LayerSpecialEvent::update(int deltaTime) {
  Layer::update(deltaTime);
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

void LayerSpecialEvent::persistRunnerStorage() {
  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  auto& persisted = stateManager->getState().world.specialEventStorage;
  persisted = runner.storage;
  runner::clearTmpStorageKeys(persisted);
}

void LayerSpecialEvent::closeLayer() {
  talkKeyboardScroll.clearHeld();
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
      stateManager->getActionData(),
      new state::actions::UiRemoveLayer(bmin::String(LAYER_ID.data(), LAYER_ID.size())),
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

  if (key == "Escape") {
    closeLayer();
    return;
  }

  if (talkKeyboardScroll.onKeyDown(key)) {
    return;
  }

  if (key == "Return" || key == "Keypad Enter" || key == "space") {
    talkKeyboardScroll.clearHeld();
    beginKeyboardContinuePress();
    return;
  }

  if (key.size() == 1 && key[0] >= '1' && key[0] <= '9') {
    talkKeyboardScroll.clearHeld();
    const auto choiceIndex = static_cast<int>(key[0] - '1');
    beginKeyboardChoicePress(choiceIndex);
  }
}

void LayerSpecialEvent::onKeyUp(std::string_view key, int /*keyCode*/) {
  talkKeyboardScroll.onKeyUp(key);
}

} // namespace layers
