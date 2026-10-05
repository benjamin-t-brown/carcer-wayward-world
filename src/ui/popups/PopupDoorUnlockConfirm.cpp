#include "PopupDoorUnlockConfirm.h"
#include "sdl2w/L10n.h"
#include "ui/components/ConfirmModal.h"
#include "ui/elements/buttons/ButtonGroup.h"
#include "ui/observers/ActionObserver.hpp"
#include "actions/navigation/UiConfirmDoorUnlock.hpp"
#include "actions/navigation/UiRemoveLayer.hpp"

namespace ui {

PopupDoorUnlockConfirm::PopupDoorUnlockConfirm(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = true;
}

PopupDoorUnlockConfirm::~PopupDoorUnlockConfirm() = default;

void PopupDoorUnlockConfirm::setProps(const PopupDoorUnlockConfirmProps& _props) {
  props = _props;
  build();
}

PopupDoorUnlockConfirmProps& PopupDoorUnlockConfirm::getProps() { return props; }

const PopupDoorUnlockConfirmProps& PopupDoorUnlockConfirm::getProps() const {
  return props;
}

void PopupDoorUnlockConfirm::build() {
  children.clear();

  auto modal = new ConfirmModal(window, this);
  modal->setId("confirmModal");
  modal->setPos(style.x, style.y);
  modal->setScale(style.scale);

  modal->setProps(ConfirmModalProps{
      .title = TRANSLATE("Locked Door"),
      .message = props.message,
      .dismissOnly = props.dismissOnly,
  });

  auto* buttonGroup = modal->getButtonGroup();
  if (buttonGroup) {
    if (props.dismissOnly) {
      buttonGroup->addObserverToButtonAtIndex(
          0,
          ui::makeActionObserver<state::actions::UiRemoveLayer>(
              state::LayerId::DoorUnlockConfirm));
    } else {
      buttonGroup->addObserverToButtonAtIndex(
          0,
          ui::makeActionObserver<state::actions::UiRemoveLayer>(
              state::LayerId::DoorUnlockConfirm));
      buttonGroup->addObserverToButtonAtIndex(
          1,
          ui::makeActionObserver<state::actions::UiConfirmDoorUnlock>(props.worldX,
                                                                      props.worldY));
    }
  }

  auto [modalW, modalH] = modal->getDims();
  style.width = modalW / style.scale;
  style.height = modalH / style.scale;

  addChild(bmin::UniquePtr<ui::UiElement>(modal));
}

void PopupDoorUnlockConfirm::render(int dt) { UiElement::render(dt); }

} // namespace ui
