#pragma once

#include "../UiElement.h"
#include "lib/sdl2w/L10n.h"
#include "lib/Types.h"

namespace ui {

class ButtonGroup;

struct ConfirmModalProps {
  String title = String(TRANSLATE("Confirm"));
  String message;
  String confirmButtonLabel = String(TRANSLATE("Yes"));
  String cancelButtonLabel = String(TRANSLATE("No"));
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
