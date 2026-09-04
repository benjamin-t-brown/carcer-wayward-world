export module carcer.actions.ui.UiCommitEquipRunes;
export import carcer.state;

export {

namespace state {

namespace actions {

/** Keep live equipped-rune edits and close the Equip Runes layer. */
class UiCommitEquipRunes : public AbstractAction {
  void act() override { LayerManagerInterface::closeLayer(LayerId::EquipRunes); }
};

} // namespace actions

} // namespace state

} // export
