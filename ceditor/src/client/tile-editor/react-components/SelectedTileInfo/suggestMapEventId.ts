import { randomId } from '../../../utils/mathUtils';

/** `{mapName}_{randomId}`, skipping ids that already exist. */
export function suggestMapEventId(
  mapName: string,
  existingEventIds?: ReadonlySet<string>
): string {
  const trimmed = mapName.trim();
  const prefix = trimmed ? `${trimmed}_` : '';
  for (let attempt = 0; attempt < 8; attempt++) {
    const id = `${prefix}${randomId()}`;
    if (!existingEventIds?.has(id)) {
      return id;
    }
  }
  return `${prefix}${randomId()}`;
}
