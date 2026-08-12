#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "model/templates/RuneTypes.h"
#include "ui/UiElement.h"

namespace ui {

struct MinipageEquipRunesSlot {
  bool filled = false;
  bmin::String iconSprite;
};

struct MinipageEquipRunesRow {
  model::RuneType type = model::RuneType::HEAT;
  bmin::String iconSprite;
  int availableCount = 0;
  int equippedCount = 0;
};

struct MinipageEquipRunesProps {
  int width = 0;
  int height = 0;
  bmin::String characterPlayerId;
  bmin::String characterPlayerLabel;
  bmin::DynArray<MinipageEquipRunesSlot> runeSlots;
  bmin::DynArray<MinipageEquipRunesRow> runeRows;
};

/** ModalSmall editor: equipped strip + available rune +/- list; Okay commits, X cancels. */
class MinipageEquipRunes : public UiElement {
private:
  MinipageEquipRunesProps props;

  static constexpr int contentPadding = 8;
  static constexpr int nameRowHeight = 28;
  static constexpr int runeSlotSize = 24;
  static constexpr int runeSlotGap = 4;
  static constexpr int runeSlotIconSize = 24;
  static constexpr float runeSlotIconScale = 1.f;
  static constexpr int equippedRowHeight = 40;
  static constexpr int gridCols = 2;
  // Match PageCharacter +/- ButtonIcon size (32).
  static constexpr int adjustButtonSize = 32;
  static constexpr int cellHeight = adjustButtonSize;
  static constexpr int cellGapX = 8;
  static constexpr int cellGapY = 4;
  static constexpr int cellInnerGap = 2;
  static constexpr int countTextWidth = 20;
  static constexpr int footerButtonWidth = 100;
  static constexpr int modalBorderWidth = 2;
  static constexpr int modalHeaderHeight = 80;

  int runeCellContentWidth() const;
  int contentInnerWidth() const;
  int contentInnerHeight() const;
  int modalWidth() const;
  int modalHeight() const;

  void addEquippedSlots(UiElement* parent, int x, int y, int width);
  void addRuneGrid(UiElement* parent, int x, int y, int width);

public:
  MinipageEquipRunes(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~MinipageEquipRunes() override = default;

  void setProps(const MinipageEquipRunesProps& _props);
  MinipageEquipRunesProps& getProps();
  const MinipageEquipRunesProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
