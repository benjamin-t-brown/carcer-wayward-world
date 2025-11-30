import { drawRect } from '../utils/draw';
import { getTransform } from './seEditorState';
import {
  GameEvent,
  GameEventChildExec,
  GameEventChildType,
} from '../types/assets';
import { EditorNodeExec } from './cmpts/ExecNodeComponent';
import { EditorStateSE } from './seEditorState';

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
  // const gameEvent = dataInterface.getEditorState().gameEvent;
  // if (gameEvent && gameEvent.children) {
  //   for (const child of gameEvent.children) {
  //     const { exits } = getAnchorCoordinates(child);
  //     const nextChildren = getNodeChildren(child);

  //     for (let i = 0; i < exits.length; i++) {
  //       const nextChildId = nextChildren[i];
  //       const nextChild = gameEvent.children.find((c) => c.id === nextChildId);
  //       const startX = exits[i].x * newScale;
  //       const startY = exits[i].y * newScale;

  //       if (nextChildId && nextChild) {
  //         const { entrance } = getAnchorCoordinates(nextChild);

  //         const endX = entrance.x * newScale;
  //         const endY = entrance.y * newScale;

  //         ctx.save();
  //         ctx.strokeStyle = 'rgba(255, 255, 255, 0.3)';
  //         ctx.lineWidth = 10 * newScale;
  //         ctx.beginPath();
  //         // if (child.eventChildType === GameEventChildType.SWITCH) {
  //         //   console.log('move to', exits.length, i, startX, startY);
  //         // }
  //         ctx.moveTo(startX, startY);
  //         ctx.lineTo(endX, endY);
  //         ctx.stroke();
  //         ctx.restore();

  //         ctx.save();
  //         ctx.fillStyle = 'white';
  //         const ANCHOR_RADIUS = 4;
  //         // entrance anchor
  //         ctx.beginPath();
  //         ctx.arc(endX, endY, ANCHOR_RADIUS * newScale, 0, Math.PI * 2);
  //         ctx.fill();
  //         ctx.restore();
  //       }

  //       // exit anchor
  //       ctx.save();
  //       ctx.fillStyle = 'white';
  //       const ANCHOR_RADIUS = 4;
  //       // Anchor at start (right side of parent node)
  //       ctx.beginPath();
  //       ctx.arc(startX, startY, ANCHOR_RADIUS * newScale, 0, Math.PI * 2);
  //       ctx.fill();
  //       ctx.restore();
  //     }
  //   }
  // }

  // Render nodes
  const hoveredNodeId = dataInterface.getEditorState().hoveredNodeId;
  const hoveredCloseButtonNodeId =
    dataInterface.getEditorState().hoveredCloseButtonNodeId;
  const selectedNodeIds = dataInterface.getEditorState().selectedNodeIds;
  const linkingExitIndex = dataInterface.getEditorState().linking.exitIndex;

  // Find child node IDs of the hovered node (nodes that the hovered node points to)
  const childNodeIds = new Set<string>();
  // if (hoveredNodeId && gameEvent && gameEvent.children) {
  //   const hoveredNode = gameEvent.children.find((c) => c.id === hoveredNodeId);
  //   if (hoveredNode && hoveredNode.eventChildType === GameEventChildType.EXEC) {
  //     const hoveredExecNode = hoveredNode as GameEventChildExec;
  //     if (hoveredExecNode.next && hoveredExecNode.next !== '') {
  //       childNodeIds.add(hoveredExecNode.next);
  //     }
  //   }
  // }

  // Find parent node IDs (nodes that point to the hovered node)
  const parentNodeIds = new Set<string>();
  // if (hoveredNodeId && gameEvent && gameEvent.children) {
  //   for (const child of gameEvent.children) {
  //     if (child.eventChildType === GameEventChildType.EXEC) {
  //       const execNode = child as GameEventChildExec;
  //       if (execNode.next === hoveredNodeId) {
  //         parentNodeIds.add(child.id);
  //       }
  //     }
  //   }
  // }

  // if (gameEvent && gameEvent.children) {
  //   for (const child of gameEvent.children) {
  //     const hoveredExitIndex =
  //       hoveredExitAnchor?.nodeId === child.id
  //         ? hoveredExitAnchor.exitIndex
  //         : undefined;
  //     const childLinkingExitIndex =
  //       isLinking && linkingSourceNodeId === child.id
  //         ? linkingExitIndex
  //         : undefined;
  //     renderNode(child, newScale, ctx, {
  //       isHovered: child.id === hoveredNodeId,
  //       isCloseButtonHovered: child.id === hoveredCloseButtonNodeId,
  //       isSelected: selectedNodeIds.has(child.id),
  //       isChildOfHovered: childNodeIds.has(child.id),
  //       isParentOfHovered: parentNodeIds.has(child.id),
  //       hoveredExitIndex: hoveredExitIndex,
  //       linkingExitIndex: childLinkingExitIndex,
  //     });
  //   }
  // }

  for (const node of dataInterface.getEditorState().editorNodes) {
    node.update();
    node.renderConnectors(ctx, newScale);
  }

  for (const node of dataInterface.getEditorState().editorNodes) {
    node.render(ctx, newScale, {
      isHovered: node.id === hoveredNodeId,
      isCloseButtonHovered: node.id === hoveredCloseButtonNodeId,
      isSelected: selectedNodeIds.has(node.id),
      isChildOfHovered: childNodeIds.has(node.id),
      isParentOfHovered: parentNodeIds.has(node.id),
      // hoveredExitIndex: hoveredExitAnchor?.exitIndex,
      hoveredExitIndex: dataInterface.getEditorState().hoveredExitAnchor?.exitIndex,
      linkingExitIndex: linkingExitIndex,
    });
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

  // Render linking mode text or copy feedback in bottom left (screen coordinates)
  const editorState = dataInterface.getEditorState();
  ctx.save();
  ctx.resetTransform(); // Use screen coordinates
  ctx.fillStyle = 'white';
  ctx.font = '14px arial';
  ctx.textAlign = 'left';
  ctx.textBaseline = 'bottom';

  const padding = 10;
  const canvasHeight = dataInterface.getCanvas().height;

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
      canvasHeight - padding
    );
  }

  ctx.restore();
};
