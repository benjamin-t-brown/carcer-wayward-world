export module carcer.actions.ui.UiCancelEquipRunes;
export import carcer.state;

export {

namespace state {

namespace actions {

/** Restore equipped runes from the open-editor snapshot and close the layer. */
class UiCancelEquipRunes : public AbstractAction {
  void act() override { LayerManagerInterface::cancelEquipRunes(); }
};

} // namespace actions

} // namespace state

} // export
