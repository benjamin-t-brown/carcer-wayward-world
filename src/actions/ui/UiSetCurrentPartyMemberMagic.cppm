export module carcer.actions.ui.UiSetCurrentPartyMemberMagic;
export import carcer.state;
import sdl2w;
#include "macros.h"

export {

namespace state {

namespace actions {

class UiSetCurrentPartyMemberMagic : public AbstractAction {
  int partyMemberMagicIndex = 0;

  void act() override {
    auto& localState = *state;

    int nextIndex = partyMemberMagicIndex;
    if (partyMemberMagicIndex < 0 ||
        partyMemberMagicIndex >= static_cast<int>(localState.player.party.size())) {
      nextIndex = 0;
      LOG(WARN) << "UiSetCurrentPartyMemberMagic::act: index out of range "
                << partyMemberMagicIndex << " "
                << localState.player.party.size() << LOG_ENDL;
    }
    localState.player.currentPartyMemberMagicIndex = nextIndex;
  }

public:
  explicit UiSetCurrentPartyMemberMagic(int _partyMemberMagicIndex)
      : partyMemberMagicIndex(_partyMemberMagicIndex) {}
};

} // namespace actions

} // namespace state

} // export
