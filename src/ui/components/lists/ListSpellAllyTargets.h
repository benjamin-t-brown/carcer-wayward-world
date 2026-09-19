#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "ui/UiElement.h"

namespace ui {

struct ListSpellAllyTargetsEntry {
  bmin::String id;
  bmin::String label;
  bmin::String iconSprite;
};

struct ListSpellAllyTargetsProps {
  bmin::DynArray<ListSpellAllyTargetsEntry> allies;
  bmin::String casterId;
  bmin::String spellId;
  int width = 0;
  int lineHeight = 32;
  int lineGap = 2;
  int paddingTop = 4;
  int paddingBottom = 12;
};

class ListSpellAllyTargets : public UiElement {
private:
  ListSpellAllyTargetsProps props;

  static constexpr float iconScale = 1.f;
  static constexpr int labelGapAfterIcon = 12;
  static constexpr int maxShortcutItems = 26;
  static constexpr int shortcutColumnWidth = 14;
  static constexpr int shortcutGapAfterLetter = 6;

  UiElement* createAllyElement(const ListSpellAllyTargetsEntry& ally, int index);
  static bmin::String shortcutLetterForIndex(int index);

public:
  ListSpellAllyTargets(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListSpellAllyTargets() override = default;

  void setProps(const ListSpellAllyTargetsProps& props);
  const ListSpellAllyTargetsProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
