#include "FloatingNotificationSection.h"
#include "actions/navigation/UiPushFloatingNotification.hpp"
#include "actions/navigation/UiRemoveFloatingNotification.hpp"
#include "actions/navigation/UiGiveInventoryItem.hpp"
#include "actions/navigation/UiToggleEquipInventoryItem.hpp"
#include "ui/components/FloatingNotification.h"

namespace ui {

FloatingNotificationSection::FloatingNotificationSection(sdl2w::Window* _window,
                                                           UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = false;

  // Action subscriptions still refresh immediately when those actions run;
  // render also polls floatingNotificationRevision for inline pushes / expiry.
  subscribeAction<state::ActionEvent::UiPushFloatingNotification>(
      [this](const auto&, const state::State& state) {
        syncFromState(state);
      });
  subscribeAction<state::ActionEvent::UiRemoveFloatingNotification>(
      [this](const auto&, const state::State& state) {
        syncFromState(state);
      });
  subscribeAction<state::ActionEvent::UiToggleEquipInventoryItem>(
      [this](const auto&, const state::State& state) {
        syncFromState(state);
      });
  subscribeAction<state::ActionEvent::UiGiveInventoryItem>(
      [this](const auto&, const state::State& state) {
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
  syncedRevision = state.uiState.floatingNotificationRevision;
  children.clear();

  for (const auto& notification : state.uiState.floatingNotifications) {
    auto floatingNotification = new FloatingNotification(window, this);
    floatingNotification->setId("notification-" + notification.id);
    floatingNotification->setProps(FloatingNotificationProps{
        .id = notification.id,
        .message = notification.message,
        .type = notification.type,
    });
    addChild(bmin::UniquePtr<ui::UiElement>(floatingNotification));
  }

  layoutNotifications();
}

void FloatingNotificationSection::syncIfRevisionChanged() {
  if (!hasStateManager()) {
    return;
  }
  const auto& state = getStateManager()->getState();
  if (state.uiState.floatingNotificationRevision == syncedRevision) {
    return;
  }
  syncFromState(state);
}

void FloatingNotificationSection::layoutNotifications() {
  const int pad = props.margin;
  const int originX = props.originX + pad;
  int currentY = props.originY + pad;
  int maxWidth = 0;
  int stackHeight = 0;

  // Anchor at the top-left of the game area; oldest first, newer below.
  for (const auto& child : children) {
    if (stackHeight > 0) {
      stackHeight += props.notificationGap;
      currentY += props.notificationGap;
    }
    auto [childWidth, childHeight] = child->getDims();
    maxWidth = std::max(maxWidth, childWidth);
    stackHeight += childHeight;

    child->setPos(originX, currentY);
    child->build();
    currentY += childHeight;
  }

  style.x = originX;
  style.y = props.originY + pad;
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

void FloatingNotificationSection::render(int dt) {
  syncIfRevisionChanged();
  UiElement::render(dt);
}

} // namespace ui
