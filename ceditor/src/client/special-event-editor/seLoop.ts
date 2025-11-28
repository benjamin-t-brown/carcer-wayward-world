import { drawRect } from '../utils/draw';
import { getTransform } from './seEditorEvents';
import {
  GameEvent,
  GameEventChildExec,
  GameEventChildType,
} from '../types/assets';
import { renderExecNode } from './nodeRendering/renderExecNode';
import { EditorStateSE } from './seEditorState';
import { getNodeBounds } from './nodeRendering/nodeHelpers';

const getColors = () => {
  return {
    BACKGROUND1: 'black',
    BACKGROUND2: '#243F72',
    TEXT: 'white',
  };
};

export const loop = (
  dataInterface: {
    getCanvas: () => HTMLCanvasElement;
    getEditorState: () => EditorStateSE;
  },
  ms: number
) => {
  const ctx = dataInterface.getCanvas().getContext('2d');
  if (!ctx) {
    return;
  }

  if (!dataInterface.getEditorState().gameEvent) {
    return;
  }

  ctx.clearRect(
    0,
    0,
    dataInterface.getCanvas().width,
    dataInterface.getCanvas().height
  );
  drawRect(
    0,
    0,
    dataInterface.getCanvas().width,
    dataInterface.getCanvas().height,
    getColors().BACKGROUND2,
    false,
    ctx
  );

  const { x, y, scale } = getTransform();

  //background rect
  ctx.save();
  ctx.translate(x, y);
  ctx.scale(scale, scale);
  ctx.translate(
    dataInterface.getCanvas().width / 2,
    dataInterface.getCanvas().height / 2
  );
  ctx.translate(
    -dataInterface.getEditorState().zoneWidth / 2,
    -dataInterface.getEditorState().zoneHeight / 2
  );
  drawRect(
    0,
    0,
    dataInterface.getEditorState().zoneWidth,
    dataInterface.getEditorState().zoneHeight,
    getColors().BACKGROUND1,
    false,
    ctx
  );
  ctx.restore();

  const newScale = scale;

  const focalX = dataInterface.getCanvas().width / 2;
  const focalY = dataInterface.getCanvas().height / 2;

  const offsetX = focalX - (newScale / scale) * (focalX - x);
  const offsetY = focalY - (newScale / scale) * (focalY - y);

  ctx.save();
  ctx.translate(offsetX, offsetY);
  ctx.translate(
    (dataInterface.getCanvas().width * newScale) / 2,
    (dataInterface.getCanvas().height * newScale) / 2
  );
  ctx.translate(
    -(dataInterface.getEditorState().zoneWidth * newScale) / 2,
    -(dataInterface.getEditorState().zoneHeight * newScale) / 2
  );

  // Render connections between nodes (lines)
  const gameEvent = dataInterface.getEditorState().gameEvent;
  if (gameEvent && gameEvent.children) {
    for (const child of gameEvent.children) {
      const [nodeWidth] = getNodeBounds(child);
      if (child.eventChildType === GameEventChildType.EXEC) {
        const execNode = child as GameEventChildExec;
        // Check if this node has a next property pointing to another node
        if (execNode.next && execNode.next !== '') {
          const nextNode = gameEvent.children.find((c) => c.id === execNode.next);
          if (nextNode) {
            // Draw line from right side of parent to left side of child
            const startX = (execNode.x + nodeWidth) * newScale;
            const startY = (execNode.y + execNode.h / 2) * newScale;
            const endX = nextNode.x * newScale;
            const endY = (nextNode.y + nextNode.h / 2) * newScale;

            ctx.save();
            ctx.strokeStyle = 'rgba(255, 255, 255, 0.3)'; // Semi-transparent white
            ctx.lineWidth = 10 / newScale;
            ctx.beginPath();
            ctx.moveTo(startX, startY);
            ctx.lineTo(endX, endY);
            ctx.stroke();
            ctx.restore();

            // Draw anchor dots at connection points
            ctx.save();
            ctx.fillStyle = 'white';
            const ANCHOR_RADIUS = 4;
            // Anchor at start (right side of parent node)
            ctx.beginPath();
            ctx.arc(startX, startY, ANCHOR_RADIUS / newScale, 0, Math.PI * 2);
            ctx.fill();
            // Anchor at end (left side of child node)
            ctx.beginPath();
            ctx.arc(endX, endY, ANCHOR_RADIUS / newScale, 0, Math.PI * 2);
            ctx.fill();
            ctx.restore();
          }
        }
      }
    }
  }

  // Render nodes
  const hoveredNodeId = dataInterface.getEditorState().hoveredNodeId;
  const hoveredCloseButtonNodeId = dataInterface.getEditorState().hoveredCloseButtonNodeId;
  const selectedNodeIds = dataInterface.getEditorState().selectedNodeIds;
  if (gameEvent && gameEvent.children) {
    for (const child of gameEvent.children) {
      if (child.eventChildType === GameEventChildType.EXEC) {
        renderExecNode(
          child as GameEventChildExec,
          child.x,
          child.y,
          newScale,
          ctx,
          {
            isHovered: child.id === hoveredNodeId,
            isCloseButtonHovered: child.id === hoveredCloseButtonNodeId,
            isSelected: selectedNodeIds.has(child.id),
          }
        );
      }
    }
  }
  
  // Draw selection rectangle (already in the correct transform context)
  const selectionRect = dataInterface.getEditorState().selectionRect;
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
      (maxY - minY) * newScale
    );
    ctx.fillStyle = 'rgba(78, 201, 176, 0.1)';
    ctx.fillRect(
      minX * newScale,
      minY * newScale,
      (maxX - minX) * newScale,
      (maxY - minY) * newScale
    );
    ctx.setLineDash([]);
  }

  ctx.restore();
};
