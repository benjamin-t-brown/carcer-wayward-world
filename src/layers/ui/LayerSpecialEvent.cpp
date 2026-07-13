#include "LayerSpecialEvent.h"
#include "sdl2w/Logger.h"
#include "state/StateManager.h"
#include "state/actions/ui/UiRemoveLayer.hpp"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/elements/TextLine.h"
#include "ui/observers/ObserverRemoveLayer.hpp"
#include "ui/observers/ObserverSpecialEventChoice.hpp"
#include "ui/elements/buttons/ButtonGroup.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/observers/ObserverSpecialEventContinue.hpp"
#include "ui/pages/PageModalEvent.h"
#include "ui/pages/PageTalkChoice.h"
#include <algorithm>

namespace layers {

namespace {

constexpr int TALK_CHOICE_AREA_HEIGHT = 120;
constexpr int MODAL_WIDTH = 500;

ui::PageTalkChoiceProps buildTalkProps(runner::SpecialEventRunner& runner,
                                       int windowWidth,
                                       int windowHeight) {
  ui::PageTalkChoiceProps props;
  props.width = windowWidth;
  props.height = windowHeight;
  props.choiceAreaHeight = TALK_CHOICE_AREA_HEIGHT;
  props.title = runner.gameEvent.title.empty() ? runner.gameEvent.id : runner.gameEvent.title;
  props.portraitSpriteName = runner.gameEvent.icon;
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
    props.choices.pushBack(item);
  }
  return props;
}

ui::PageModalEventProps buildModalProps(runner::SpecialEventRunner& runner,
                                        int windowWidth,
                                        int windowHeight) {
  ui::PageModalEventProps props;
  props.width = MODAL_WIDTH;
  props.height = std::min(windowHeight - 50, 420);
  props.title = runner.gameEvent.title.empty() ? runner.gameEvent.id : runner.gameEvent.title;
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
    props.choices.pushBack(item);
  }
  props.showContinueButton =
      props.choices.empty() && !runner.displayText.empty() && !runner.getNextNodeId().empty();
  return props;
}

} // namespace

LayerSpecialEvent::LayerSpecialEvent(
    sdl2w::Window* _window,
    const model::GameEvent& gameEvent,
    const bmin::Map<bmin::String, model::GameEvent>& gameEvents)
    : Layer(_window, LAYER_ID),
      runner({}, gameEvent, gameEvents),
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
    pageTalkChoice->setProps(buildTalkProps(
        runner, static_cast<int>(windowWidth / scale), static_cast<int>(windowHeight / scale)));
    addUiElement(pageTalkChoice);
  } else {
    auto modalProps = buildModalProps(
        runner, static_cast<int>(windowWidth / scale), static_cast<int>(windowHeight / scale));
    auto pageModalEvent = new ui::PageModalEvent(window);
    pageModalEvent->setId("eventPage");
    pageModalEvent->setPos((static_cast<int>(windowWidth / scale) - modalProps.width) / 2,
                           (static_cast<int>(windowHeight / scale) - modalProps.height) / 2);
    pageModalEvent->setScale(scale);
    pageModalEvent->setProps(modalProps);
    addUiElement(pageModalEvent);
  }

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);

  runnerInterface.startEvent();
  syncUi();
}

void LayerSpecialEvent::syncUi() {
  if (runner.gameEvent.eventType == model::GameEventType::TALK) {
    auto* pageTalkChoice = getUiElement<ui::PageTalkChoice>("eventPage");
    if (!pageTalkChoice) {
      return;
    }
    auto [windowWidth, windowHeight] = window->getDims();
    pageTalkChoice->setProps(buildTalkProps(
        runner, static_cast<int>(windowWidth), static_cast<int>(windowHeight)));
    attachChoiceObservers();
    return;
  }

  auto* pageModalEvent = getUiElement<ui::PageModalEvent>("eventPage");
  if (!pageModalEvent) {
    return;
  }
  auto [windowWidth, windowHeight] = window->getDims();
  auto modalProps = buildModalProps(runner, static_cast<int>(windowWidth), static_cast<int>(windowHeight));
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
    auto* choiceSection = pageTalkChoice->getChildById("choiceSection");
    if (!choiceSection) {
      return;
    }
    for (size_t i = 0; i < choiceSection->getChildren().size(); i++) {
      auto* child = choiceSection->getChildren()[i].get();
      if (!child) {
        continue;
      }
      child->addEventObserver(new ui::ObserverSpecialEventChoice(this, static_cast<int>(i)));
    }
    return;
  }

  auto* pageModalEvent = getUiElement<ui::PageModalEvent>("eventPage");
  if (!pageModalEvent) {
    return;
  }
  auto* textSection = pageModalEvent->getChildById("modal");
  if (!textSection) {
    return;
  }
  textSection = textSection->getChildById("textSection");
  if (!textSection) {
    return;
  }
  int choiceIndex = 0;
  for (size_t i = 0; i < textSection->getChildren().size(); i++) {
    auto* child = textSection->getChildren()[i].get();
    if (!child) {
      continue;
    }
    const auto& childId = child->getId();
    if (!childId.startsWith("choice")) {
      continue;
    }
    child->addEventObserver(new ui::ObserverSpecialEventChoice(this, choiceIndex));
    choiceIndex++;
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
  if (continuePressRemainingMs > 0) {
    return;
  }
  if (!runner.displayTextChoices.empty()) {
    return;
  }

  if (auto* button = findModalContinueButton()) {
    button->isActive = true;
    continuePressRemainingMs = 120;
    return;
  }

  onContinue();
}

void LayerSpecialEvent::onChoiceSelected(int choiceIndex) {
  if (eventFinished) {
    return;
  }
  runnerInterface.selectChoice(choiceIndex);
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

  runnerInterface.continueEvent();
  if (runner.getNextNodeId().empty() && runner.displayTextChoices.empty()) {
    closeLayer();
    return;
  }
  needsSyncUi = true;
}

void LayerSpecialEvent::update(int deltaTime) {
  Layer::update(deltaTime);

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

void LayerSpecialEvent::closeLayer() {
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

void LayerSpecialEvent::onKeyDown(std::string_view key, int /*keyCode*/) {
  if (getState() != LayerState::ON) {
    return;
  }

  if (key == "Escape") {
    closeLayer();
    return;
  }

  if (key == "Return" || key == "space") {
    beginKeyboardContinuePress();
    return;
  }

  if (key.size() == 1 && key[0] >= '1' && key[0] <= '9') {
    const auto choiceIndex = static_cast<int>(key[0] - '1');
    if (choiceIndex >= 0 &&
        static_cast<size_t>(choiceIndex) < runner.displayTextChoices.size()) {
      onChoiceSelected(choiceIndex);
    }
  }
}

} // namespace layers
