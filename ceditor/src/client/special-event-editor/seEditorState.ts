import { GameEvent } from "../types/assets";

export class SpecialEventEditorState {
  selectedEventId = '';
  zoneWidth = 750;
  zoneHeight = 750;
  // maps: Record<string, EditorStateMap>;
}

const specialEventEditorState = new SpecialEventEditorState();

export const getEditorState = () => specialEventEditorState;
export const updateEditorState = (state: Partial<SpecialEventEditorState>) => {
  Object.assign(specialEventEditorState, { ...getEditorState(), ...state });
  (window as any).reRenderSpecialEventEditor();
};
export const updateEditorStateNoReRender = (
  state: Partial<SpecialEventEditorState>
) => {
  Object.assign(specialEventEditorState, { ...getEditorState(), ...state });
};
(window as any).specialEventEditorState = specialEventEditorState;
