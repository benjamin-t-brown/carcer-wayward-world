#pragma once

#include "bmin/DynArray.h"
#include "bmin/StringInterop.h"
#include "bmin/UniquePtr.h"
#include "layers/Layer.h"
#include "ui/UiElement.h"

namespace layers {

/** Optional visual specialization; the Layer orchestration base remains UI-free. */
class UiLayer : public Layer {
protected:
  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> uiElements;

public:
  using Layer::Layer;

  void addUiElement(ui::UiElement* element);

  template <typename T> T* getUiElement(std::string_view elementId) {
    for (auto& element : uiElements) {
      if (bmin::toStringView(element->getId()) == elementId) {
        return dynamic_cast<T*>(element.get());
      }
    }
    return nullptr;
  }

  void onMouseDown(int x, int y, int button) override;
  void onMouseUp(int x, int y, int button) override;
  void onMouseHover(int x, int y) override;
  void onMouseWheel(int x, int y, int direction) override;
  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers
