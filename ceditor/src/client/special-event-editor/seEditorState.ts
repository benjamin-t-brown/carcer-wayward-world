export interface SpecialEventEditorState {
  selectedEventId: string;
  zoneWidth: number;
  zoneHeight: number;
  // maps: Record<string, EditorStateMap>;
}

const specialEventEditorState: SpecialEventEditorState = {
  selectedEventId: '',
  zoneWidth: 10000,
  zoneHeight: 10000,
};
export const getEditorState = () => specialEventEditorState;
export const updateEditorState = (state: Partial<SpecialEventEditorState>) => {
  Object.assign(specialEventEditorState, { ...getEditorState(), ...state });
  (window as any).reRenderTileEditor();
};
export const updateEditorStateNoReRender = (
  state: Partial<SpecialEventEditorState>
) => {
  Object.assign(specialEventEditorState, { ...getEditorState(), ...state });
};
(window as any).specialEventEditorState = specialEventEditorState;
