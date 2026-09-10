import type {
  MapDocument,
  MapTileReference,
} from '../../../core/domain/maps/index.js';
import type { Viewport } from './Viewport.js';

export interface MapOverlayBlock {
  readonly document: MapDocument;
  readonly originX: number;
  readonly originY: number;
}

export interface MapOverlayOptions {
  readonly layer: number;
  readonly canvasWidth: number;
  readonly canvasHeight: number;
  readonly showLabels?: boolean;
}

interface OverlayKind {
  readonly key: keyof MapDocument['placements'];
  readonly color: string;
  readonly symbol: string;
  label(placement: MapTileReference): string;
}

const KINDS: readonly OverlayKind[] = [
  { key: 'characters', color: '#62d8a5', symbol: 'C', label: named },
  { key: 'items', color: '#f6c85f', symbol: 'I', label: named },
  { key: 'markers', color: '#93c5fd', symbol: 'M', label: named },
  { key: 'eventTriggers', color: '#c084fc', symbol: 'E', label: eventLabel },
  { key: 'travelTriggers', color: '#fb7185', symbol: 'T', label: travelLabel },
  {
    key: 'tileOverrides',
    color: '#f97316',
    symbol: 'O',
    label: () => 'override',
  },
  {
    key: 'lightSources',
    color: '#fde047',
    symbol: 'L',
    label: () => 'light',
  },
];

/** Draws sparse map metadata without materializing per-tile overlay objects. */
export class MapOverlayRenderer {
  draw(
    context: CanvasRenderingContext2D,
    viewport: Viewport,
    blocks: Iterable<MapOverlayBlock>,
    options: MapOverlayOptions,
  ): number {
    let drawn = 0;
    context.save();
    context.font = '11px monospace';
    context.textBaseline = 'top';
    for (const block of blocks) {
      for (const kind of KINDS) {
        const placements = block.document.placements[kind.key];
        if (!placements) continue;
        for (const placement of placements) {
          if ((placement.l ?? 0) !== options.layer) continue;
          const position = block.document.coordinatesOf(placement.i ?? 0);
          if (!position) continue;
          const left = viewport.worldToScreenX(
            block.originX + position.x * block.document.spriteWidth,
          );
          const top = viewport.worldToScreenY(
            block.originY + position.y * block.document.spriteHeight,
          );
          const right = viewport.worldToScreenX(
            block.originX + (position.x + 1) * block.document.spriteWidth,
          );
          const bottom = viewport.worldToScreenY(
            block.originY + (position.y + 1) * block.document.spriteHeight,
          );
          if (
            right < 0 ||
            bottom < 0 ||
            left > options.canvasWidth ||
            top > options.canvasHeight
          )
            continue;
          const size = Math.max(8, Math.min(16, Math.abs(right - left) * 0.42));
          context.fillStyle = kind.color;
          context.fillRect(
            Math.round(left) + 2,
            Math.round(top) + 2,
            size,
            size,
          );
          context.fillStyle = '#09090b';
          context.fillText(
            kind.symbol,
            Math.round(left) + 4,
            Math.round(top) + 3,
          );
          if (options.showLabels) {
            context.fillStyle = kind.color;
            context.fillText(
              kind.label(placement),
              Math.round(left) + 2,
              Math.round(bottom) - 12,
            );
          }
          drawn += 1;
        }
      }
    }
    context.restore();
    return drawn;
  }
}

function stringField(placement: MapTileReference, key: string): string {
  const value = placement[key];
  return typeof value === 'string' ? value : '';
}

function named(placement: MapTileReference): string {
  return stringField(placement, 'name') || '(unnamed)';
}

function eventLabel(placement: MapTileReference): string {
  return stringField(placement, 'eventId') || '(event)';
}

function travelLabel(placement: MapTileReference): string {
  return stringField(placement, 'destinationMapName') || '(travel)';
}
