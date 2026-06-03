#pragma once

#include "model/instances/CharacterPlayer.h"
#include "model/stats/CharacterDerivedStats.h"
#include "state/DatabaseInterface.h"
#include "ui/UiElement.h"

namespace ui {

class SectionScrollable;

struct PageCharacterProps {
  model::CharacterPlayer* characterPlayer = nullptr;
};

struct PageCharacterStatRowEntry {
  std::string label;
  std::string helpDescription;
  int value = 0;
  std::string valueText;
};

struct PageCharacterStatRowSectionArgs {
  std::string title;
  std::vector<PageCharacterStatRowEntry> rows;
  int width  = 0;
  int y = 0;
  bool showModButtons = false;
  bool buttonMinusDisabled = false;
  bool buttonPlusDisabled = false;
};

// PageCharacter - character stats and level-up UI inside a modal.
class PageCharacter : public UiElement, public state::DatabaseInterface {
private:
  PageCharacterProps props;

  void addDerivedStatSections(SectionScrollable* scrollable,
                              int sectionContentW,
                              int& yAgg,
                              const model::CharacterDerivedStats& derivedStats);

public:
  PageCharacter(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PageCharacter() override = default;

  void setProps(const PageCharacterProps& _props);
  PageCharacterProps& getProps();
  const PageCharacterProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  UiElement* buildStatSection(const PageCharacterStatRowSectionArgs& section);

  void build() override;
  void render(int dt) override;
};

} // namespace ui
