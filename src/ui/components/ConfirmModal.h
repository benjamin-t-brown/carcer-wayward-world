#pragma once

#include "../UiElement.h"
#include "sdl2w/L10n.h"
#include "bmin/String.h"

namespace ui {

class ButtonGroup;

struct ConfirmModalProps {
  bmin::String title = bmin::String(TRANSLATE("Confirm"));
  bmin::String message;
  bmin::String confirmButtonLabel = bmin::String(TRANSLATE("Yes"));
  bmin::String cancelButtonLabel = bmin::String(TRANSLATE("No"));
};

class ConfirmModal : public UiElement {
  ConfirmModalProps props;

  ButtonGroup* buttonGroup = nullptr;

public:
  ConfirmModal(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ConfirmModal() override = default;

  void setProps(const ConfirmModalProps& _props);
  ConfirmModalProps& getProps();
  const ConfirmModalProps& getProps() const;

  ButtonGroup* getButtonGroup();
  const ButtonGroup* getButtonGroup() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
