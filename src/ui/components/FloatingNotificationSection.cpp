#include "FloatingNotificationSection.h"
#include "state/actions/ui/UiPushFloatingNotification.hpp"
#include "state/actions/ui/UiRemoveFloatingNotification.hpp"
#include "state/actions/ui/UiGiveInventoryItem.hpp"
#include "state/actions/ui/UiToggleEquipInventoryItem.hpp"
#include "ui/components/FloatingNotification.h"

namespace ui {

FloatingNotificationSection::FloatingNotificationSection(sdl2w::Window* _window,
                                                           UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = false;

  subscribeAction<state::actions::UiPushFloatingNotification>(
      [this](const state::actions::UiPushFloatingNotification&, const state::State& state) {
        syncFromState(state);
      });
  subscribeAction<state::actions::UiRemoveFloatingNotification>(
      [this](const state::actions::UiRemoveFloatingNotification&, const state::State& state) {
        syncFromState(state);
      });
  subscribeAction<state::actions::UiToggleEquipInventoryItem>(
      [this](const state::actions::UiToggleEquipInventoryItem&, const state::State& state) {
        syncFromState(state);
      });
  subscribeAction<state::actions::UiGiveInventoryItem>(
      [this](const state::actions::UiGiveInventoryItem&, const state::State& state) {
        syncFromState(state);
      });

  if (hasStateManager()) {
    syncFromState(getStateManager()->getState());
  }
}

FloatingNotificationSection::~FloatingNotificationSection() {
  if (hasStateManager()) {
    getStateManager()->getActionBus().unsubscribe(this);
  }
}

void FloatingNotificationSection::setProps(const FloatingNotificationSectionProps& _props) {
  props = _props;
  build();
}

const FloatingNotificationSectionProps& FloatingNotificationSection::getProps() const {
  return props;
}

void FloatingNotificationSection::syncFromState(const state::State& state) {
  children.clear();

  for (const auto& notification : state.uiState.floatingNotifications) {
    auto floatingNotification = new FloatingNotification(window, this);
    floatingNotification->setId("notification-" + notification.id);
    floatingNotification->setProps(FloatingNotificationProps{
        .id = notification.id,
        .message = notification.message,
        .type = notification.type,
    });
    addChild(floatingNotification);
  }

  layoutNotifications();
}

void FloatingNotificationSection::layoutNotifications() {
  auto [windowWidth, windowHeight] = window->getDims();

  int stackHeight = 0;
  int maxWidth = 0;
  for (const auto& child : children) {
    if (stackHeight > 0) {
      stackHeight += props.notificationGap;
    }
    auto [childWidth, childHeight] = child->getDims();
    maxWidth = std::max(maxWidth, childWidth);
    stackHeight += childHeight;
  }

  // Anchor the stack at the bottom of the screen; newest notification sits lowest.
  int currentY = windowHeight - props.bottomMargin;
  for (auto it = children.rbegin(); it != children.rend(); ++it) {
    auto& child = *it;
    auto [childWidth, childHeight] = child->getDims();

    currentY -= childHeight;
    child->setPos((windowWidth - childWidth) / 2, currentY);
    child->build();

    currentY -= props.notificationGap;
  }

  style.x = (windowWidth - maxWidth) / 2;
  style.y = windowHeight - props.bottomMargin - stackHeight;
  style.width = maxWidth;
  style.height = stackHeight;
}

void FloatingNotificationSection::build() {
  if (hasStateManager()) {
    syncFromState(getStateManager()->getState());
  } else {
    layoutNotifications();
  }
}

void FloatingNotificationSection::render(int dt) { UiElement::render(dt); }

} // namespace ui
