import { GameEventChildType, SENode } from '../types/assets';
import { drawRect } from '../utils/draw';
import { Connector } from './cmpts/Connector';
import { EditorNodeCloseButton } from './cmpts/EditorNodeCloseButton';
import { EditorStateSE } from './seEditorState';

export interface RenderNodeArgs {
  isHovered: boolean;
  isCloseButtonHovered?: boolean;
  isSelected?: boolean;
  isChildOfHovered?: boolean;
  isParentOfHovered?: boolean;
  hoveredExitIndex?: number; // Exit index that is hovered for this node
  linkingExitIndex?: number | null; // Exit index that is being linked for this node
}

export class EditorNode {
  editorState: EditorStateSE;
  id: string;
  x: number = 0;
  y: number = 0;
  width: number = 300;
  height: number = 20;
  borderSize: number = 2;
  padding: number = 6;
  type: GameEventChildType;
  borderColor: string = '#aaa';
  borderColorSelected: string = '#ff0000';
  borderColorHovered: string = '#fff';
  bgColor: string = '#808080';
  bgColorHovered: string = '#a0a0a0';

  exits: Connector[] = [];
  closeButton: EditorNodeCloseButton;

  constructor(seNode: SENode, editorState: EditorStateSE) {
    this.editorState = editorState;
    this.id = seNode.id;
    this.x = seNode.x;
    this.y = seNode.y;
    this.type = seNode.eventChildType;
    this.closeButton = new EditorNodeCloseButton(this);
  }

  toSENode() {
    return {
      id: this.id,
      x: this.x,
      y: this.y,
      eventChildType: this.type,
      h: this.height,
    } as SENode;
  }

  isPointInBounds(x: number, y: number) {
    return (
      x >= this.x &&
      x <= this.x + this.width &&
      y >= this.y &&
      y <= this.y + this.height
    );
  }

  isPointInCloseButtonBounds(x: number, y: number) {
    return this.closeButton.isColliding(x, y);
  }

  getAnchorCollidingWithPoint(x: number, y: number) {
    for (const exit of this.exits) {
      if (exit.isColliding(x, y)) {
        return exit;
      }
    }
    return undefined;
  }

  getConnectorLineCollidingWithPoint(x: number, y: number) {
    for (const exit of this.exits) {
      if (exit.isLineColliding(x, y)) {
        return exit;
      }
    }
    return undefined;
  }

  calculateHeight(ctx: CanvasRenderingContext2D) {
    this.height = 20;
  }

  getBounds() {
    return {
      x: this.x,
      y: this.y,
      width: this.width,
      height: this.height,
    };
  }

  getEntrancePos() {
    return {
      x: this.x,
      y: this.y + this.height / 2,
    };
  }

  getChildrenIds() {
    return this.exits
      .map((exit) => exit.toNodeId)
      .filter((nodeId) => Boolean(nodeId));
  }

  updateExitLink(nextNodeId: string, exitIndex = 0) {
    this.exits[exitIndex].toNodeId = nextNodeId;
  }

  clearExitLinks() {
    for (const exit of this.exits) {
      exit.toNodeId = '';
    }
  }

  update() {
    this.closeButton.update();
    for (const exit of this.exits) {
      exit.update();
    }
  }

  renderAnchor(
    ctx: CanvasRenderingContext2D,
    x: number,
    y: number,
    scale: number,
    args: {
      isLinking: boolean;
      isHovered: boolean;
      isEntrance: boolean;
      isConnected: boolean;
    }
  ) {
    // Render exit anchors as partially transparent red circles
    const EXIT_ANCHOR_RADIUS = 10; // 10px radius
    const HOVERED_EXIT_ANCHOR_RADIUS = 14; // 14px radius when hovered

    const isHovered = args.isHovered;
    const isLinking = args.isLinking;
    const isEntrance = args.isEntrance;
    let isConnected = args.isConnected;
    let radius = isHovered ? HOVERED_EXIT_ANCHOR_RADIUS : EXIT_ANCHOR_RADIUS;

    ctx.save();
    // Exit coordinates are in world space, convert to canvas space
    const exitX = x * scale;
    const exitY = y * scale;

    // Draw circle - white when linking, brighter when hovered, green if connected, red otherwise
    if (isEntrance) {
      ctx.fillStyle = 'rgba(255, 255, 255, 0.5)';
      radius = 5;
    } else if (isLinking) {
      ctx.fillStyle = 'rgba(255, 255, 255, 0.9)'; // White when linking
    } else if (isHovered) {
      if (isConnected) {
        ctx.fillStyle = 'rgba(100, 255, 100, 0.8)'; // Brighter green when hovered and connected
      } else {
        ctx.fillStyle = 'rgba(255, 100, 100, 0.8)'; // Brighter red when hovered and not connected
      }
    } else {
      if (isConnected) {
        ctx.fillStyle = 'rgba(0, 255, 0, 0.5)'; // Green when connected
      } else {
        ctx.fillStyle = 'rgba(255, 0, 0, 0.5)'; // Red when not connected
      }
    }
    ctx.beginPath();
    ctx.arc(exitX, exitY, radius * scale, 0, Math.PI * 2);
    ctx.fill();
    ctx.restore();
  }

  // separated so it can render below all other nodes
  renderConnectors(ctx: CanvasRenderingContext2D, scale: number) {
    for (const connector of this.exits) {
      connector.render(ctx, scale);
    }
  }

  render(ctx: CanvasRenderingContext2D, scale: number, args: RenderNodeArgs) {
    const borderWidth = args.isSelected ? 3 : this.borderSize;
    const borderColor = args.isSelected
      ? this.borderColorSelected
      : args.isHovered
      ? this.borderColorHovered
      : this.borderColor;
    drawRect(
      this.x * scale,
      this.y * scale,
      this.width * scale,
      this.height * scale,
      borderColor,
      false,
      ctx
    );

    drawRect(
      (this.x + borderWidth) * scale,
      (this.y + borderWidth) * scale,
      this.width * scale - borderWidth * 2 * scale,
      this.height * scale - borderWidth * 2 * scale,
      this.bgColor,
      false,
      ctx
    );

    for (const connector of this.exits) {
      this.renderAnchor(ctx, connector.startX, connector.startY, scale, {
        isLinking:
          this.editorState.linking.isLinking &&
          this.editorState.linking.sourceNodeId === this.id &&
          connector.exitIndex === this.editorState.linking.exitIndex,
        isHovered:
          this.editorState.hoveredExitAnchor === connector &&
          connector.exitIndex === args.hoveredExitIndex,
        isEntrance: false,
        isConnected: connector.toNodeId !== '',
      });
    }

    const entrancePos = this.getEntrancePos();
    this.renderAnchor(ctx, entrancePos.x, entrancePos.y, scale, {
      isLinking: false,
      isHovered: false,
      isEntrance: true,
      isConnected: this.exits.length > 0,
    });

    this.closeButton.render(ctx, scale, {
      isHovered: Boolean(args.isCloseButtonHovered),
      isActive: false,
    });
  }
}
