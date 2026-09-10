import type { AbilityTemplate } from '../types/ability';
import {
  sanitizeMapGridTemplates,
  type CharacterTemplate,
  type ItemTemplate,
  type MapGridTemplate,
  type TilesetTemplate,
} from '../types/assets';
import { trimStrings } from './jsonUtils';

export function prepareAbilitiesForSave(
  abilities: AbilityTemplate[],
): AbilityTemplate[] {
  return trimStrings(abilities).sort((a, b) => a.name.localeCompare(b.name));
}

export function prepareItemsForSave(items: ItemTemplate[]): ItemTemplate[] {
  return trimStrings(items).sort((a, b) => {
    const nameComparison = a.name.localeCompare(b.name);
    return nameComparison === 0
      ? a.label.localeCompare(b.label)
      : nameComparison;
  });
}

export function prepareCharactersForSave(
  characters: CharacterTemplate[],
): CharacterTemplate[] {
  return trimStrings(characters).sort((a, b) => {
    const nameComparison = a.name.localeCompare(b.name);
    return nameComparison === 0
      ? a.label.localeCompare(b.label)
      : nameComparison;
  });
}

export function prepareTilesetsForSave(
  tilesets: TilesetTemplate[],
): TilesetTemplate[] {
  return trimStrings(tilesets).sort((a, b) => a.name.localeCompare(b.name));
}

export function prepareMapGridsForSave(
  mapGrids: MapGridTemplate[],
): MapGridTemplate[] {
  return sanitizeMapGridTemplates(trimStrings(mapGrids)).sort((a, b) =>
    a.name.localeCompare(b.name),
  );
}

export function prepareTemplateRecordsForSave<T>(
  records: T[],
  getId: (record: T) => string,
  compare?: (a: T, b: T) => number,
): T[] {
  const prepared = trimStrings(records);
  const compareRecords =
    compare ?? ((a: T, b: T) => getId(a).localeCompare(getId(b)));
  return prepared.sort(compareRecords);
}
