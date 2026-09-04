module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.ui.popups.PopupDropConfirm;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.UiElement;
import sdl2w;
import carcer.actions.ui.UiDropInventoryItem;
import carcer.ui.ObserverRemoveLayer;
import carcer.state;
import carcer.ui.components.ConfirmModal;
#include "macros.h"

export {

// --- from ui/popups/PopupDropConfirm.h ---
namespace ui {

struct PopupDropConfirmProps {
  bmin::String characterPlayerId;
  bmin::String itemId;
  bmin::String itemLabel;
};

class PopupDropConfirm : public UiElement {
  PopupDropConfirmProps props;

public:
  PopupDropConfirm(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PopupDropConfirm() override;

  void setProps(const PopupDropConfirmProps& _props);
  PopupDropConfirmProps& getProps();
  const PopupDropConfirmProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverDropInventoryItem.hpp ---
namespace ui {

class ObserverDropInventoryItem : public ui::UiEventObserver,
                                  public state::StateManagerInterface {
  bmin::String characterPlayerId;
  bmin::String itemId;

public:
  ObserverDropInventoryItem(const bmin::String& _characterPlayerId, const bmin::String& _itemId)
      : characterPlayerId(_characterPlayerId), itemId(_itemId) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

} // export

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
  //       bmin::makeUnique<ObserverRemoveLayer>(state::LayerId::DropConfirm);
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
      0, new ObserverRemoveLayer(state::LayerId::DropConfirm));

  auto [modalW, modalH] = modal->getDims();
  style.width = modalW / style.scale;
  style.height = modalH / style.scale;

  addChild(modal);
}

void PopupDropConfirm::render(int dt) { UiElement::render(dt); }

} // namespace ui

namespace ui {

void ObserverDropInventoryItem::onClick(int mouseX, int mouseY, int button) {
    LOG(INFO) << "ObserverDropInventoryItem::onClick item=" << itemId << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::UiDropInventoryItem(characterPlayerId, itemId),
        0);
  }

} // namespace ui
