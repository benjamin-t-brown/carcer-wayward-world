#include "LayerSpecialEvent.h"
#include "actions/navigation/UiRemoveLayer.hpp"
#include "game/SpecialEventPresenter.h"
#include "in3/EventRunnerHelpers.h"
#include "state/State.hpp"
#include "state/StateManager.h"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/pages/PageModalEvent.h"
#include "ui/pages/PageTalkChoice.h"
#include "ui/pages/specialEventViewMapping.h"

namespace layers {

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
  const auto view = game::SpecialEventPresenter::view(
      runner, talkHistory, getDatabase(), runnerInterface);

  if (isTalkEvent()) {
    auto pageTalkChoice = new ui::PageTalkChoice(window);
    pageTalkChoice->setId("eventPage");
    pageTalkChoice->setPos(0, 0);
    pageTalkChoice->setScale(scale);
    pageTalkChoice->setProps(ui::SpecialEventViewMapping::toTalkProps(
        view,
        static_cast<int>(windowWidth / scale),
        static_cast<int>(windowHeight / scale)));
    addUiElement(bmin::UniquePtr<ui::UiElement>(pageTalkChoice));
  } else {
    auto pageModalEvent = new ui::PageModalEvent(window);
    pageModalEvent->setId("eventPage");
    pageModalEvent->setPos(0, 0);
    pageModalEvent->setScale(scale);
    pageModalEvent->setProps(ui::SpecialEventViewMapping::toModalProps(
        view,
        static_cast<int>(windowWidth / scale),
        static_cast<int>(windowHeight / scale)));
    addUiElement(bmin::UniquePtr<ui::UiElement>(pageModalEvent));
  }

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(bmin::UniquePtr<ui::UiElement>(floatingNotificationSection));

  runnerInterface.startEvent();
  syncUi();
}

bool LayerSpecialEvent::isTalkEvent() const {
  return runner.gameEvent.eventType == model::GameEventType::TALK;
}

ui::PageTalkChoice* LayerSpecialEvent::talkPage() {
  return getUiElement<ui::PageTalkChoice>("eventPage");
}

ui::PageModalEvent* LayerSpecialEvent::modalPage() {
  return getUiElement<ui::PageModalEvent>("eventPage");
}

void LayerSpecialEvent::syncUi() {
  auto [windowWidth, windowHeight] = window->getDims();
  const auto view = game::SpecialEventPresenter::view(
      runner, talkHistory, getDatabase(), runnerInterface);

  if (auto* pageTalkChoice = talkPage()) {
    pageTalkChoice->setProps(ui::SpecialEventViewMapping::toTalkProps(
        view, static_cast<int>(windowWidth), static_cast<int>(windowHeight)));
    return;
  }

  auto* pageModalEvent = modalPage();
  if (!pageModalEvent) {
    return;
  }
  pageModalEvent->setPos(0, 0);
  pageModalEvent->setProps(ui::SpecialEventViewMapping::toModalProps(
      view, static_cast<int>(windowWidth), static_cast<int>(windowHeight)));
}

void LayerSpecialEvent::onChoiceSelected(int choiceIndex) {
  if (runnerInterface.isFinished()) {
    return;
  }
  stopEventPageKeyboardChrome();
  if (isTalkEvent()) {
    game::SpecialEventPresenter::commitTalkChoice(
        runner, talkHistory, choiceIndex, getDatabase());
  } else {
    game::SpecialEventPresenter::consumeModalNotices(runner);
  }
  runnerInterface.selectChoice(choiceIndex);
  if (runnerInterface.isFinished()) {
    closeLayer();
    return;
  }
  needsSyncUi = true;
}

void LayerSpecialEvent::onContinue() {
  stopEventPageKeyboardChrome();
  if (runnerInterface.isFinished()) {
    closeLayer();
    return;
  }

  if (!runner.displayTextChoices.empty()) {
    return;
  }

  if (isTalkEvent()) {
    game::SpecialEventPresenter::commitTalkCurrent(
        runner, talkHistory, getDatabase());
  } else {
    game::SpecialEventPresenter::consumeModalNotices(runner);
  }
  runnerInterface.continueEvent();
  if (runnerInterface.isFinished()) {
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

void LayerSpecialEvent::stopEventPageKeyboardChrome() {
  if (auto* page = talkPage()) {
    page->stopKeyboardChrome();
    return;
  }
  if (auto* page = modalPage()) {
    page->stopKeyboardChrome();
  }
}

void LayerSpecialEvent::updateEventPageKeyboardChrome(int deltaTime) {
  if (auto* page = talkPage()) {
    page->updateKeyboardChrome(deltaTime);
    return;
  }
  if (auto* page = modalPage()) {
    page->updateKeyboardChrome(deltaTime);
  }
}

void LayerSpecialEvent::closeLayer() {
  stopEventPageKeyboardChrome();
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

void LayerSpecialEvent::onKeyDown(std::string_view key, int /*keyCode*/) {
  if (getState() != LayerState::ON) {
    return;
  }

  // Talk must be exited via choices / the modal close control — not Escape.
  if (key == "Escape") {
    if (!isTalkEvent()) {
      closeLayer();
    }
    return;
  }

  if (auto* page = talkPage()) {
    page->onKeyDown(key);
    return;
  }
  if (auto* page = modalPage()) {
    page->onKeyDown(key);
  }
}

void LayerSpecialEvent::onKeyUp(std::string_view key, int /*keyCode*/) {
  if (auto* page = talkPage()) {
    page->onKeyUp(key);
    return;
  }
  if (auto* page = modalPage()) {
    page->onKeyUp(key);
  }
}

void LayerSpecialEvent::update(int deltaTime) {
  UiLayer::update(deltaTime);
  updateEventPageKeyboardChrome(deltaTime);

  if (needsSyncUi) {
    needsSyncUi = false;
    syncUi();
  }
}

} // namespace layers
