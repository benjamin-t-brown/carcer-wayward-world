import {
  sanitizeMapGridTemplates,
  type CarcerMapTemplate,
  type MapGridTemplate,
} from '../types/assets';
import { trimStrings } from './jsonUtils';
import { assignMapToGridCell, renameMapInGrids } from './mapGridIndex';

export interface GridCellAssignment {
  gridName: string;
  cellX: number;
  cellY: number;
}

export interface MapDatabaseCandidates {
  maps: CarcerMapTemplate[];
  mapGrids: MapGridTemplate[];
}

export interface GridMapCreationCandidates extends MapDatabaseCandidates {
  createdMap: CarcerMapTemplate;
}

export function prepareGridMapCreationCandidates(
  maps: CarcerMapTemplate[],
  mapGrids: MapGridTemplate[],
  preparedMap: CarcerMapTemplate,
  assignment: GridCellAssignment,
): GridMapCreationCandidates {
  const createdMap = trimStrings(preparedMap);
  return {
    maps: [...maps, createdMap],
    mapGrids: sanitizeMapGridTemplates(
      trimStrings(
        assignMapToGridCell(
          mapGrids,
          assignment.gridName,
          assignment.cellX,
          assignment.cellY,
          createdMap.name,
        ),
      ),
    ),
    createdMap,
  };
}

export interface MapRenameCandidates extends MapDatabaseCandidates {
  oldName: string;
  newName: string;
  renamed: boolean;
}

export function prepareMapRenameCandidates(
  maps: CarcerMapTemplate[],
  mapGrids: MapGridTemplate[],
  currentMapName: string,
  updatedMap: CarcerMapTemplate,
): MapRenameCandidates | null {
  const oldName = currentMapName.trim();
  const preparedMap = trimStrings(updatedMap);
  const newName = preparedMap.name;
  const mapIndex = maps.findIndex((map) => map.name === currentMapName);
  if (mapIndex < 0) {
    return null;
  }

  const nextMaps = [...maps];
  nextMaps[mapIndex] = preparedMap;
  const renamed = Boolean(oldName && newName && oldName !== newName);

  return {
    maps: nextMaps,
    mapGrids: renamed
      ? sanitizeMapGridTemplates(
          trimStrings(renameMapInGrids(mapGrids, oldName, newName)),
        )
      : mapGrids,
    oldName,
    newName,
    renamed,
  };
}

/** Build the persisted maps value after the canvas layer has been committed. */
export function prepareMapsSaveCandidate(
  maps: CarcerMapTemplate[],
  flushedCanvasMap?: CarcerMapTemplate,
): CarcerMapTemplate[] {
  const candidate = flushedCanvasMap
    ? maps.map((map) =>
        map.name === flushedCanvasMap.name ? flushedCanvasMap : map,
      )
    : maps;
  return trimStrings(candidate);
}
