import { ASSET_IDS, type AssetId } from '../database/assetRegistry.js';
import type { DatabaseSnapshot } from '../database/types.js';
import { buildReferenceIndex } from '../references/referenceIndex.js';
import type { DatabaseReference } from '../references/types.js';
import type {
  ValidationIssue,
  ValidationResult,
  ValidationSeverity,
} from './types.js';

const IDENTITY_KEYS: Record<AssetId, 'id' | 'name'> = {
  statusEffects: 'name',
  abilities: 'name',
  items: 'name',
  spells: 'name',
  characters: 'name',
  maps: 'name',
  mapGrids: 'name',
  tilesets: 'name',
  specialEvents: 'id',
};

type JsonRecord = Record<string, unknown>;

function isObject(value: unknown): value is JsonRecord {
  return typeof value === 'object' && value !== null && !Array.isArray(value);
}

function nonEmptyText(value: unknown): string | null {
  return typeof value === 'string' && value.trim() ? value.trim() : null;
}

function isPositiveInteger(value: unknown): value is number {
  return Number.isInteger(value) && Number(value) > 0;
}

function makeIssue(
  severity: ValidationSeverity,
  code: string,
  message: string,
  path: string,
  assetId?: AssetId,
  recordId?: string,
): ValidationIssue {
  return { severity, code, message, path, assetId, recordId };
}

function issueForReference(reference: DatabaseReference): ValidationIssue {
  const severity: ValidationSeverity =
    reference.integrity === 'soft' ? 'warning' : 'error';
  const scope = reference.target.scope ? ` in ${reference.target.scope}` : '';
  return makeIssue(
    severity,
    reference.integrity === 'structural'
      ? 'event.target.missing'
      : 'reference.unresolved',
    `Unresolved ${reference.relation} reference "${reference.target.id}"${scope}.`,
    reference.source.path,
    reference.source.assetId,
    reference.source.recordId,
  );
}

function validateCollections(
  snapshot: unknown,
  issues: ValidationIssue[],
): Partial<Record<AssetId, unknown[]>> {
  const collections: Partial<Record<AssetId, unknown[]>> = {};
  if (!isObject(snapshot)) {
    issues.push(
      makeIssue(
        'error',
        'database.root.object',
        'Database snapshot must be an object.',
        '$',
      ),
    );
    return collections;
  }

  for (const assetId of ASSET_IDS) {
    const value = snapshot[assetId];
    if (!Array.isArray(value)) {
      issues.push(
        makeIssue(
          'error',
          'collection.root.array',
          `${assetId} must be a JSON array.`,
          assetId,
          assetId,
        ),
      );
      continue;
    }
    collections[assetId] = value;

    const identityKey = IDENTITY_KEYS[assetId];
    const firstIndexById = new Map<string, number>();
    value.forEach((record, index) => {
      const path = `${assetId}[${index}]`;
      if (!isObject(record)) {
        issues.push(
          makeIssue(
            'error',
            'record.object',
            `${assetId} record ${index} must be an object.`,
            path,
            assetId,
          ),
        );
        return;
      }

      const id = nonEmptyText(record[identityKey]);
      if (!id) {
        issues.push(
          makeIssue(
            'error',
            'record.identity.required',
            `Record requires a non-empty string ${identityKey}.`,
            `${path}.${identityKey}`,
            assetId,
          ),
        );
        return;
      }

      const firstIndex = firstIndexById.get(id);
      if (firstIndex !== undefined) {
        issues.push(
          makeIssue(
            'error',
            'record.identity.duplicate',
            `Duplicate ${identityKey} "${id}"; first used at ${assetId}[${firstIndex}].`,
            `${path}.${identityKey}`,
            assetId,
            id,
          ),
        );
      } else {
        firstIndexById.set(id, index);
      }
    });
  }
  return collections;
}

function validateMaps(
  records: unknown[] | undefined,
  issues: ValidationIssue[],
) {
  records?.forEach((value, mapIndex) => {
    if (!isObject(value)) {
      return;
    }
    const path = `maps[${mapIndex}]`;
    const recordId = nonEmptyText(value.name) ?? undefined;
    const width = value.width;
    const height = value.height;
    if (!isPositiveInteger(width)) {
      issues.push(
        makeIssue(
          'error',
          'map.width.positiveInteger',
          'Map width must be a positive integer.',
          `${path}.width`,
          'maps',
          recordId,
        ),
      );
    }
    if (!isPositiveInteger(height)) {
      issues.push(
        makeIssue(
          'error',
          'map.height.positiveInteger',
          'Map height must be a positive integer.',
          `${path}.height`,
          'maps',
          recordId,
        ),
      );
    }

    const layers = value.layers;
    const layerNumbers: number[] = [];
    if (!Array.isArray(layers)) {
      issues.push(
        makeIssue(
          'error',
          'map.layers.array',
          'Map layers must be an array.',
          `${path}.layers`,
          'maps',
          recordId,
        ),
      );
    } else {
      const seen = new Set<number>();
      layers.forEach((layer, layerIndex) => {
        if (!Number.isInteger(layer)) {
          issues.push(
            makeIssue(
              'error',
              'map.layer.integer',
              'Map layer must be an integer.',
              `${path}.layers[${layerIndex}]`,
              'maps',
              recordId,
            ),
          );
          return;
        }
        const layerNumber = Number(layer);
        layerNumbers.push(layerNumber);
        if (seen.has(layerNumber)) {
          issues.push(
            makeIssue(
              'error',
              'map.layer.duplicate',
              `Map layer ${layerNumber} is declared more than once.`,
              `${path}.layers[${layerIndex}]`,
              'maps',
              recordId,
            ),
          );
        }
        seen.add(layerNumber);
        if (layerNumber < 0) {
          issues.push(
            makeIssue(
              'warning',
              'map.layer.negative',
              `Layer ${layerNumber} is preserved, but the current C++ loader ignores negative layers.`,
              `${path}.layers[${layerIndex}]`,
              'maps',
              recordId,
            ),
          );
        }
      });
    }

    if (!isObject(value.tiles)) {
      issues.push(
        makeIssue(
          'error',
          'map.tiles.object',
          'Map tiles must be an object keyed by layer.',
          `${path}.tiles`,
          'maps',
          recordId,
        ),
      );
      return;
    }

    const expectedLength =
      isPositiveInteger(width) && isPositiveInteger(height)
        ? width * height * 2
        : null;
    const keys = new Set([
      ...layerNumbers.map(String),
      ...Object.keys(value.tiles),
    ]);
    for (const layerKey of keys) {
      const graphics = value.tiles[layerKey];
      const layerPath = `${path}.tiles[${JSON.stringify(layerKey)}]`;
      if (!/^-?\d+$/.test(layerKey)) {
        issues.push(
          makeIssue(
            'error',
            'map.tiles.layerKey',
            `Tile layer key "${layerKey}" is not an integer.`,
            layerPath,
            'maps',
            recordId,
          ),
        );
      } else if (
        Number(layerKey) < 0 &&
        !layerNumbers.includes(Number(layerKey))
      ) {
        issues.push(
          makeIssue(
            'warning',
            'map.layer.negative',
            `Layer ${layerKey} is preserved, but the current C++ loader ignores negative layers.`,
            layerPath,
            'maps',
            recordId,
          ),
        );
      }
      if (!Array.isArray(graphics)) {
        issues.push(
          makeIssue(
            'error',
            'map.tiles.array',
            `Map layer ${layerKey} must be a dense graphic array.`,
            layerPath,
            'maps',
            recordId,
          ),
        );
        continue;
      }
      if (expectedLength !== null && graphics.length !== expectedLength) {
        issues.push(
          makeIssue(
            'error',
            'map.tiles.length',
            `Map layer ${layerKey} has ${graphics.length} values; expected ${expectedLength}.`,
            layerPath,
            'maps',
            recordId,
          ),
        );
      }
      if (graphics.some((entry) => !Number.isInteger(entry))) {
        issues.push(
          makeIssue(
            'error',
            'map.tiles.integer',
            `Map layer ${layerKey} graphics must contain only integers.`,
            layerPath,
            'maps',
            recordId,
          ),
        );
      }
    }

    const placementFields = [
      'characters',
      'items',
      'markers',
      'eventTriggers',
      'travelTriggers',
      'tileOverrides',
      'lightSources',
    ];
    for (const field of placementFields) {
      if (value[field] !== undefined && !Array.isArray(value[field])) {
        issues.push(
          makeIssue(
            'error',
            'map.placements.array',
            `Map ${field} must be an array.`,
            `${path}.${field}`,
            'maps',
            recordId,
          ),
        );
      }
    }
  });
}

function validateMapGrids(
  records: unknown[] | undefined,
  issues: ValidationIssue[],
) {
  records?.forEach((value, gridIndex) => {
    if (!isObject(value)) {
      return;
    }
    const path = `mapGrids[${gridIndex}]`;
    const recordId = nonEmptyText(value.name) ?? undefined;
    for (const field of [
      'gridWidth',
      'gridHeight',
      'mapWidth',
      'mapHeight',
    ] as const) {
      if (!isPositiveInteger(value[field])) {
        issues.push(
          makeIssue(
            'error',
            'mapGrid.dimension.positiveInteger',
            `${field} must be a positive integer.`,
            `${path}.${field}`,
            'mapGrids',
            recordId,
          ),
        );
      }
    }
    if (!Array.isArray(value.cells)) {
      issues.push(
        makeIssue(
          'error',
          'mapGrid.cells.array',
          'Map-grid cells must be a two-dimensional array.',
          `${path}.cells`,
          'mapGrids',
          recordId,
        ),
      );
      return;
    }
    if (
      isPositiveInteger(value.gridHeight) &&
      value.cells.length !== value.gridHeight
    ) {
      issues.push(
        makeIssue(
          'error',
          'mapGrid.cells.height',
          `Map-grid has ${value.cells.length} rows; expected ${value.gridHeight}.`,
          `${path}.cells`,
          'mapGrids',
          recordId,
        ),
      );
    }
    value.cells.forEach((row, rowIndex) => {
      if (!Array.isArray(row)) {
        issues.push(
          makeIssue(
            'error',
            'mapGrid.cells.row',
            'Every map-grid row must be an array.',
            `${path}.cells[${rowIndex}]`,
            'mapGrids',
            recordId,
          ),
        );
        return;
      }
      if (
        isPositiveInteger(value.gridWidth) &&
        row.length !== value.gridWidth
      ) {
        issues.push(
          makeIssue(
            'error',
            'mapGrid.cells.width',
            `Map-grid row has ${row.length} cells; expected ${value.gridWidth}.`,
            `${path}.cells[${rowIndex}]`,
            'mapGrids',
            recordId,
          ),
        );
      }
      row.forEach((cell, columnIndex) => {
        if (typeof cell !== 'string') {
          issues.push(
            makeIssue(
              'error',
              'mapGrid.cells.string',
              'Map-grid cells must contain map names or empty strings.',
              `${path}.cells[${rowIndex}][${columnIndex}]`,
              'mapGrids',
              recordId,
            ),
          );
        }
      });
    });
  });
}

function validateEventNodes(
  records: unknown[] | undefined,
  issues: ValidationIssue[],
) {
  records?.forEach((value, eventIndex) => {
    if (!isObject(value)) {
      return;
    }
    const eventPath = `specialEvents[${eventIndex}]`;
    const eventId = nonEmptyText(value.id) ?? undefined;
    if (value.children === undefined) {
      return;
    }
    if (!Array.isArray(value.children)) {
      issues.push(
        makeIssue(
          'error',
          'event.children.array',
          'Special-event children must be an array.',
          `${eventPath}.children`,
          'specialEvents',
          eventId,
        ),
      );
      return;
    }
    const firstNodeById = new Map<string, number>();
    value.children.forEach((node, nodeIndex) => {
      const nodePath = `${eventPath}.children[${nodeIndex}]`;
      if (!isObject(node)) {
        issues.push(
          makeIssue(
            'error',
            'event.node.object',
            'Special-event node must be an object.',
            nodePath,
            'specialEvents',
            eventId,
          ),
        );
        return;
      }
      const nodeId = nonEmptyText(node.id);
      if (!nodeId) {
        issues.push(
          makeIssue(
            'error',
            'event.node.id.required',
            'Special-event node requires a non-empty string id.',
            `${nodePath}.id`,
            'specialEvents',
            eventId,
          ),
        );
        return;
      }
      const firstIndex = firstNodeById.get(nodeId);
      if (firstIndex !== undefined) {
        issues.push(
          makeIssue(
            'error',
            'event.node.id.duplicate',
            `Duplicate event node id "${nodeId}"; first used at child ${firstIndex}.`,
            `${nodePath}.id`,
            'specialEvents',
            eventId,
          ),
        );
      } else {
        firstNodeById.set(nodeId, nodeIndex);
      }
    });
  });
}

function isCompleteSnapshot(
  collections: Partial<Record<AssetId, unknown[]>>,
): collections is DatabaseSnapshot {
  return ASSET_IDS.every((assetId) => Array.isArray(collections[assetId]));
}

export function validateDatabase(snapshot: unknown): ValidationResult {
  const issues: ValidationIssue[] = [];
  const collections = validateCollections(snapshot, issues);
  validateMaps(collections.maps, issues);
  validateMapGrids(collections.mapGrids, issues);
  validateEventNodes(collections.specialEvents, issues);

  if (isCompleteSnapshot(collections)) {
    const referenceIndex = buildReferenceIndex(collections);
    issues.push(...referenceIndex.unresolved().map(issueForReference));
  }

  const errors = issues.filter((issue) => issue.severity === 'error');
  const warnings = issues.filter((issue) => issue.severity === 'warning');
  return {
    valid: errors.length === 0,
    issues,
    errors,
    warnings,
  };
}
