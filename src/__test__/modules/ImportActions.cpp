import carcer.actions;

int main() {
  state::StateManager stateManager;
  state::HeldMove heldMove;
  heldMove.isActive = true;
  stateManager.enqueueAction(
      stateManager.getActionData(), state::actions::updateHeldMove(heldMove), 0);
  stateManager.update(1);
  return stateManager.getState().uiState.heldMove.isActive ? 0 : 1;
}
