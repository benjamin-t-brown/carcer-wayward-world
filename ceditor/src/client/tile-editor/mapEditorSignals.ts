let renderDirty = true;
const dataRevisionByMapName = new Map<string, number>();

export function markRenderDirty(): void {
  renderDirty = true;
}

export function isRenderDirty(): boolean {
  return renderDirty;
}

export function clearRenderDirty(): void {
  renderDirty = false;
}

export function registerMapDataRevision(mapName: string): void {
  if (!dataRevisionByMapName.has(mapName)) {
    dataRevisionByMapName.set(mapName, 0);
  }
}

export function getMapDataRevision(mapName: string): number {
  return dataRevisionByMapName.get(mapName) ?? 0;
}

export function bumpMapDataRevision(mapName: string): void {
  const current = dataRevisionByMapName.get(mapName);
  if (current === undefined) {
    return;
  }
  dataRevisionByMapName.set(mapName, current + 1);
  markRenderDirty();
}

export function renameMapDataRevision(
  oldMapName: string,
  newMapName: string,
): void {
  const current = dataRevisionByMapName.get(oldMapName);
  if (current === undefined) {
    return;
  }
  dataRevisionByMapName.set(newMapName, current);
  dataRevisionByMapName.delete(oldMapName);
}
