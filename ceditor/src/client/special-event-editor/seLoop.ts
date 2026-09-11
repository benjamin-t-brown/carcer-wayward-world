import { drawRect } from '../utils/draw';
import type { Connector } from './cmpts/Connector';
import type { EditorNode } from './cmpts/EditorNode';
import { getTransform, type EditorStateSE } from './seEditorState';
import { planSpecialEventViewport } from './specialEventViewport';

const COLORS = {
  BACKGROUND1: 'black',
  BACKGROUND2: '#243F72',
  TEXT: 'white',
};
const getColors = () => {
  return COLORS;
};

export const loop = (
  dataInterface: {
    getCanvas: () => HTMLCanvasElement;
    getEditorState: () => EditorStateSE;
  },
  ms: number,
) => {
  const canvas = dataInterface.getCanvas();
  const editorState = dataInterface.getEditorState();
  const ctx = canvas.getContext('2d');
  if (!ctx) {
    return;
  }

  ctx.clearRect(0, 0, canvas.width, canvas.height);
  drawRect(
    0,
    0,
    canvas.width,
    canvas.height,
    getColors().BACKGROUND2,
    false,
    ctx,
  );

  const { x, y, scale } = getTransform(editorState);

  //background rect
  ctx.save();
  ctx.translate(x, y);
  ctx.scale(scale, scale);
  ctx.translate(canvas.width / 2, canvas.height / 2);
  ctx.translate(-editorState.zoneWidth / 2, -editorState.zoneHeight / 2);
  drawRect(
    0,
    0,
    editorState.zoneWidth,
    editorState.zoneHeight,
    getColors().BACKGROUND1,
    false,
    ctx,
  );
  ctx.restore();

  const newScale = scale;

  const focalX = canvas.width / 2;
  const focalY = canvas.height / 2;

  const offsetX = focalX - (newScale / scale) * (focalX - x);
  const offsetY = focalY - (newScale / scale) * (focalY - y);

  ctx.save();
  ctx.translate(offsetX, offsetY);
  ctx.translate((canvas.width * newScale) / 2, (canvas.height * newScale) / 2);
  ctx.translate(
    -(editorState.zoneWidth * newScale) / 2,
    -(editorState.zoneHeight * newScale) / 2,
  );

  // Render nodes
  const hoveredNodeId = editorState.hoveredNodeId;
  const hoveredCloseButtonNodeId = editorState.hoveredCloseButtonNodeId;
  const selectedNodeIds = editorState.selectedNodeIds;
  const linkingExitIndex = editorState.linking.exitIndex;

  // const childNodeIds = new Set<string>();
  // const parentNodeIds = new Set<string>();

  for (const node of editorState.editorNodes) {
    node.update(ms);
  }

  const viewportPlan = planSpecialEventViewport<Connector, EditorNode>({
    canvasWidth: canvas.width,
    canvasHeight: canvas.height,
    zoneWidth: editorState.zoneWidth,
    zoneHeight: editorState.zoneHeight,
    transform: {
      translateX: x,
      translateY: y,
      scale,
    },
    nodes: editorState.editorNodes,
  });

  for (const connector of viewportPlan.connectors) {
    connector.render(ctx, newScale);
  }

  for (const node of viewportPlan.nodes) {
    node.render(ctx, newScale, {
      isHovered: node.id === hoveredNodeId,
      isCloseButtonHovered: node.id === hoveredCloseButtonNodeId,
      isSelected: selectedNodeIds.has(node.id),
      isChildOfHovered: false,
      isParentOfHovered: false,
      hoveredExitIndex: editorState.hoveredExitAnchor?.exitIndex,
      linkingExitIndex: linkingExitIndex,
    });
  }

  // Draw selection rectangle (already in the correct transform context)
  const selectionRect = editorState.selectionRect;
  if (selectionRect) {
    const minX = Math.min(selectionRect.startX, selectionRect.endX);
    const maxX = Math.max(selectionRect.startX, selectionRect.endX);
    const minY = Math.min(selectionRect.startY, selectionRect.endY);
    const maxY = Math.max(selectionRect.startY, selectionRect.endY);

    ctx.strokeStyle = '#4ec9b0';
    ctx.lineWidth = 2 / newScale;
    ctx.setLineDash([5 / newScale, 5 / newScale]);
    ctx.strokeRect(
      minX * newScale,
      minY * newScale,
      (maxX - minX) * newScale,
      (maxY - minY) * newScale,
    );
    ctx.fillStyle = 'rgba(78, 201, 176, 0.1)';
    ctx.fillRect(
      minX * newScale,
      minY * newScale,
      (maxX - minX) * newScale,
      (maxY - minY) * newScale,
    );
    ctx.setLineDash([]);
  }

  ctx.restore();

  // Render linking mode text or copy feedback in bottom left (screen coordinates)
  ctx.save();
  ctx.resetTransform(); // Use screen coordinates
  ctx.fillStyle = 'white';
  ctx.font = '14px arial';
  ctx.textAlign = 'left';
  ctx.textBaseline = 'bottom';

  const padding = 10;
  const canvasHeight = canvas.height;

  if (editorState.linking.isLinking) {
    let linkText =
      'Link node: ' +
      editorState.linking.sourceNodeId +
      '(' +
      editorState.linking.exitIndex +
      ')' +
      ' -> ';
    if (editorState.hoveredNodeId) {
      linkText += editorState.hoveredNodeId;
    }
    ctx.fillText(linkText, padding, canvasHeight - padding);
  } else if (editorState.showCopyFeedback) {
    ctx.fillText(
      'Node ID copied to clipboard',
      padding,
      canvasHeight - padding,
    );
  }

  ctx.restore();
};
