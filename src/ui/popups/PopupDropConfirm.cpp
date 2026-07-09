#include "PopupDropConfirm.h"
#include "layers/ui/LayerDropConfirm.h"
#include "sdl2w/L10n.h"
#include "ui/components/ConfirmModal.h"
#include "ui/elements/buttons/ButtonGroup.h"
#include "ui/observers/ObserverDropInventoryItem.hpp"
#include "ui/observers/ObserverRemoveLayer.hpp"

namespace ui {

PopupDropConfirm::PopupDropConfirm(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = true;
}

PopupDropConfirm::~PopupDropConfirm() = default;

void PopupDropConfirm::setProps(const PopupDropConfirmProps& _props) {
  props = _props;
  //   confirmObserver =
  //       bmin::makeUnique<ObserverDropInventoryItem>(props.characterPlayerId,
  //       props.itemId);
  //   cancelObserver =
  //       bmin::makeUnique<ObserverRemoveLayer>(layers::LayerDropConfirm::LAYER_ID);
  build();
}

PopupDropConfirmProps& PopupDropConfirm::getProps() { return props; }

const PopupDropConfirmProps& PopupDropConfirm::getProps() const { return props; }

void PopupDropConfirm::build() {
  children.clear();

  auto modal = new ConfirmModal(window, this);
  modal->setId("confirmModal");
  modal->setPos(style.x, style.y);
  modal->setScale(style.scale);

  const bmin::String messageText =
      TRANSLATE("Are you sure you wish to drop ") + props.itemLabel + "?";
  modal->setProps(ConfirmModalProps{
      .title = TRANSLATE("Drop"),
      .message = messageText,
  });
  modal->getButtonGroup()->addObserverToButtonAtIndex(
      1, new ObserverDropInventoryItem(props.characterPlayerId, props.itemId));
  modal->getButtonGroup()->addObserverToButtonAtIndex(
      0, new ObserverRemoveLayer(layers::LayerDropConfirm::LAYER_ID));

  auto [modalW, modalH] = modal->getDims();
  style.width = modalW / style.scale;
  style.height = modalH / style.scale;

  addChild(modal);
}

void PopupDropConfirm::render(int dt) { UiElement::render(dt); }

} // namespace ui
