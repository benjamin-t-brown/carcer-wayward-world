import { CarcerMapTemplate, MapGridTemplate } from '../types/assets';
import { snapPixelArtPanOffset } from '../utils/draw';
import {
  findMapGridPlacement,
  getGridAdjacentSlots,
  GridAdjacentSlot,
  MapGridPlacement,
} from '../utils/mapGridIndex';

const HOTSPOT_FRACTION = 0.35;

export interface GridSlotHit {
  slot: GridAdjacentSlot;
  placement: MapGridPlacement;
}

export function getMapGridSlotDimensions(
  placement: MapGridPlacement,
  spriteWidth: number,
  spriteHeight: number,
  scale: number,
): { slotWidth: number; slotHeight: number } {
  const { grid } = placement;
  return {
    slotWidth: grid.mapWidth * spriteWidth * scale,
    slotHeight: grid.mapHeight * spriteHeight * scale,
  };
}

export function canvasToScaledMapOrigin(
  canvas: HTMLCanvasElement,
  map: CarcerMapTemplate,
  translateX: number,
  translateY: number,
  scale: number,
): { originX: number; originY: number } {
  const spriteWidth = map.spriteWidth;
  const spriteHeight = map.spriteHeight;
  const { x: panX, y: panY } = snapPixelArtPanOffset(
    translateX,
    translateY,
    scale,
    canvas.width,
    canvas.height,
    map.width,
    map.height,
    spriteWidth,
    spriteHeight,
  );

  return {
    originX:
      panX +
      (canvas.width * scale) / 2 -
      (map.width * spriteWidth * scale) / 2,
    originY:
      panY +
      (canvas.height * scale) / 2 -
      (map.height * spriteHeight * scale) / 2,
  };
}

export function getAdjacentSlotHotspotRect(
  slot: GridAdjacentSlot,
  slotWidth: number,
  slotHeight: number,
): { x: number; y: number; w: number; h: number } {
  const hotspotW = slotWidth * HOTSPOT_FRACTION;
  const hotspotH = slotHeight * HOTSPOT_FRACTION;
  const slotLeft = slot.offsetX * slotWidth;
  const slotTop = slot.offsetY * slotHeight;
  return {
    x: slotLeft + (slotWidth - hotspotW) / 2,
    y: slotTop + (slotHeight - hotspotH) / 2,
    w: hotspotW,
    h: hotspotH,
  };
}

export function findAdjacentGridSlotAtCanvasPoint(args: {
  canvasX: number;
  canvasY: number;
  canvas: HTMLCanvasElement;
  map: CarcerMapTemplate;
  mapGrids: MapGridTemplate[];
  maps: CarcerMapTemplate[];
  translateX: number;
  translateY: number;
  scale: number;
}): GridSlotHit | null {
  const mapsByName: Record<string, CarcerMapTemplate> = {};
  for (const entry of args.maps) {
    mapsByName[entry.name] = entry;
  }

  const placement = findMapGridPlacement(args.map.name, args.mapGrids);
  if (!placement) {
    return null;
  }

  const { originX, originY } = canvasToScaledMapOrigin(
    args.canvas,
    args.map,
    args.translateX,
    args.translateY,
    args.scale,
  );
  const localX = args.canvasX - originX;
  const localY = args.canvasY - originY;
  const { slotWidth, slotHeight } = getMapGridSlotDimensions(
    placement,
    args.map.spriteWidth,
    args.map.spriteHeight,
    args.scale,
  );

  for (const slot of getGridAdjacentSlots(placement, mapsByName)) {
    const hotspot = getAdjacentSlotHotspotRect(slot, slotWidth, slotHeight);
    if (
      localX >= hotspot.x &&
      localX < hotspot.x + hotspot.w &&
      localY >= hotspot.y &&
      localY < hotspot.y + hotspot.h
    ) {
      return { slot, placement };
    }
  }

  return null;
}
