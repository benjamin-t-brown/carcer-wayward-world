import { GameEventChildComment } from '../../types/assets';
import { drawText } from '../../utils/draw';
import { breakTextIntoLines, calculateHeightFromText } from '../nodeHelpers';
import { EditorNode, RenderNodeArgs } from '../EditorNode';
import { EditorStateSE } from '../seEditorState';

const NODE_TITLE_HEIGHT = 20;
const NODE_COLOR = '#222';
const BORDER_COLOR = '#666';
const BORDER_HOVER_COLOR = '#888';
const TEXT_COLOR = '#CCC';
const FONT_FAMILY = 'arial';
const NODE_WIDTH = 300;
const BORDER_WIDTH = 2;
const FONT_SIZE = 12;
const PADDING = 6;
const LINE_SPACING = 4;
const LINE_HEIGHT = 14;

export class EditorNodeComment extends EditorNode {
  comment: string = '';

  commentLines: string[] = [];
  commentHeight = 0;

  constructor(seNode: GameEventChildComment, editorState: EditorStateSE) {
    super(seNode, editorState);

    this.bgColor = NODE_COLOR;
    this.borderColor = BORDER_COLOR;
    this.borderColorHovered = BORDER_HOVER_COLOR;
    this.padding = PADDING;
    this.borderSize = BORDER_WIDTH;
    this.width = NODE_WIDTH;

    this.comment = seNode.comment ?? '';
  }

  toSENode() {
    return {
      ...super.toSENode(),
      eventChildType: 'COMMENT' as const,
      comment: this.comment,
    } as GameEventChildComment;
  }

  calculateHeight(ctx: CanvasRenderingContext2D) {
    const availableWidth = this.width - this.padding * 2;

    this.commentLines = breakTextIntoLines(
      this.comment || 'Comment',
      availableWidth,
      FONT_SIZE,
      FONT_FAMILY,
      ctx
    );

    let height = NODE_TITLE_HEIGHT + PADDING * 2 + BORDER_WIDTH * 2;
    height +=
      this.commentLines.length * LINE_HEIGHT +
      this.commentLines.length * LINE_SPACING;
    this.height = Math.max(height, 50);
  }

  update() {
    super.update();
  }

  render(ctx: CanvasRenderingContext2D, scale: number, args: RenderNodeArgs) {
    super.render(ctx, scale, args);

    const nodeX = this.x * scale;
    const nodeY = this.y * scale;

    // Draw comment text
    ctx.save();
    ctx.translate(nodeX, nodeY);
    ctx.scale(scale, scale);

    let yOffset = NODE_TITLE_HEIGHT + PADDING;
    for (const line of this.commentLines) {
      drawText(
        line,
        this.padding,
        yOffset,
        {
          color: TEXT_COLOR,
          size: FONT_SIZE,
          font: FONT_FAMILY,
          strokeColor: '',
          align: 'left',
          baseline: 'top',
        },
        ctx
      );
      yOffset += LINE_HEIGHT + LINE_SPACING;
    }

    ctx.restore();
  }
}
