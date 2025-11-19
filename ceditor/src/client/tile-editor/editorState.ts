// import { setLoopMap } from 'render/loop';
import { resetPanzoom } from './editorEvents';
// import { AppState, saveSettingsToLs } from 'state';
// import { getMapDataOptional } from 'render/mapCache';
// import {
//   tileIdToSpriteName,
//   tileIndexAndTilesetNameToTileId,
// } from 'render/tileset';
import { PaintActionType } from './paintTools';
// import { getColors } from 'style';

// export const setCurrentMap = async (
//   mapPath: string,
//   dontResetPanzoom?: boolean
// ) => {
//   const appState: AppState = (window as any).appState;
//   if (!appState) {
//     return;
//   }
//   appState.updateState('currentMapPath', mapPath);
//   saveSettingsToLs({ currentMapPath: mapPath });
//   await setLoopMap(mapPath);
//   if (!dontResetPanzoom) {
//     resetPanzoom();
//   }
// };

// export const getCurrentMapData = () => {
//   const appState: AppState = (window as any).appState;
//   return getMapDataOptional(appState.currentMapPath);
// };

export interface EditorState {
  selectedTileIndex: number;
  selectedTilesetName: string;
  currentPaintAction: PaintActionType;
}

let editorState: EditorState = {
  selectedTileIndex: -1,
  selectedTilesetName: '',
  currentPaintAction: PaintActionType.NONE,
};
export const getEditorState = () => editorState;
export const updateEditorState = (state: Partial<EditorState>) => {
  editorState = { ...getEditorState(), ...state };
};

export const getCurrentSelectedTileId = () => {
  // const appState: AppState = (window as any).appState;
  const selectedTileIndex = getEditorState().selectedTileIndex ?? -1;
  const selectedTilesetName = getEditorState().selectedTilesetName ?? '';

  if (!selectedTilesetName || selectedTileIndex === -1) {
    return 0;
  }

  // return (
  //   tileIndexAndTilesetNameToTileId(selectedTileIndex, selectedTilesetName) ?? 0
  // );
  return selectedTilesetName + '_' + selectedTileIndex;
};

// export const getCurrentSelectedTileColor = () => {
//   const selectedTileColor = getEditorState().selectedTileColor ?? '';
//   return selectedTileColor;
// };

export const setCurrentPaintAction = (paintAction: PaintActionType) => {
  updateEditorState({ currentPaintAction: paintAction });
};

export const getCurrentPaintAction = () => {
  return getEditorState().currentPaintAction;
};

export const setSelectedTileFromTileId = (tileId: number) => {
  // updateEditorState({ selectedTileIndex: tileId });
  // const spriteName = tileIdToSpriteName(tileId);
  // if (spriteName) {
  //   const arr = spriteName.split('_');
  //   const tileIndex = Number(arr[arr.length - 1]);
  //   const tilesetImage = arr.slice(0, arr.length - 1).join('_');
  //   const tilesetName = appState.tilesets.find(
  //     (t) => t.image === tilesetImage
  //   )?.name;
  //   // saveSettingsToLs({
  //   //   selectedTileIndex: tileIndex,
  //   //   selectedTilesetName: tilesetName,
  //   // });
  //   updateEditorState({ selectedTilesetName: tilesetName });
  //   updateEditorState({ selectedTileIndex: tileIndex });
  // }
};

export const setSelectedMapTileIndex = (index: number) => {
  updateEditorState({ selectedTileIndex: index });
};

// export const addLayerToRefTile = (tileInd: number, tileId: number) => {
//   const appState: AppState = (window as any).appState;
//   const currentMapPath = appState.currentMapPath;
//   const mapData = getMapDataOptional(currentMapPath);
//   if (mapData) {
//     const tileRef = mapData.ref[tileInd];
//     if (tileRef) {
//       tileRef.arr.push(tileId);
//       tileRef.opacity.push(100);
//       tileRef.color.push(getColors().TEXT);
//     }
//   }
// };

// export const removeLayerFromRefTile = (tileInd: number, layerInd: number) => {
//   const appState: AppState = (window as any).appState;
//   const currentMapPath = appState.currentMapPath;
//   const mapData = getMapDataOptional(currentMapPath);
//   if (mapData) {
//     const tileRef = mapData.ref[tileInd];
//     if (tileRef) {
//       tileRef.arr.splice(layerInd, 1);
//       tileRef.opacity.splice(layerInd, 1);
//       tileRef.color.splice(layerInd, 1);
//     }
//   }
// };

// export const updateLayerTileInRefTile = (
//   tileInd: number,
//   layerInd: number,
//   tileId: number
// ) => {
//   const appState: AppState = (window as any).appState;
//   const currentMapPath = appState.currentMapPath;
//   const mapData = getMapDataOptional(currentMapPath);
//   if (mapData) {
//     const tileRef = mapData.ref[tileInd];
//     if (tileRef) {
//       tileRef.arr[layerInd] = tileId;
//     }
//   }
// };

// export const updateLayerTileOpacity = (
//   tileInd: number,
//   layerInd: number,
//   opacity: number
// ) => {
//   const appState: AppState = (window as any).appState;
//   const currentMapPath = appState.currentMapPath;
//   const mapData = getMapDataOptional(currentMapPath);
//   if (mapData) {
//     const tileRef = mapData.ref[tileInd];
//     if (tileRef) {
//       tileRef.opacity[layerInd] = opacity;
//     }
//   }
// };

// export const updateLayerTileColor = (
//   tileInd: number,
//   layerInd: number,
//   color: string
// ) => {
//   const appState: AppState = (window as any).appState;
//   const currentMapPath = appState.currentMapPath;
//   const mapData = getMapDataOptional(currentMapPath);
//   saveSettingsToLs({ selectedTileColor: color });
//   if (mapData) {
//     const tileRef = mapData.ref[tileInd];
//     if (tileRef) {
//       tileRef.color[layerInd] = color;
//     }
//   }
// };
