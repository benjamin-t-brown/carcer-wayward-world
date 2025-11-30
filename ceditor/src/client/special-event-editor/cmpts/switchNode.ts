// import { GameEventChildSwitch, SwitchCase } from '../../types/assets';
// import { drawRect, drawText } from '../../utils/draw';
// import { renderCloseButton } from './CloseButtonSubComponent';
// import { RenderNodeArgs, getAnchorCoordinates } from '../nodeHelpers';

// const NODE_COLOR = '#141F42';
// const BORDER_COLOR = '#aaa';
// const BORDER_HOVER_COLOR = '#fff';
// const TEXT_COLOR = '#FFF';
// const FONT_FAMILY = 'courier';
// const NODE_WIDTH = 300;
// const NODE_TITLE_HEIGHT = 20;
// const BORDER_WIDTH = 2;
// const FONT_SIZE = 12;
// const PADDING = 6;
// const LINE_SPACING = 4;
// const LINE_HEIGHT = 24;

// export const getSwitchNodeDimensions = (node: GameEventChildSwitch) => {
//   return [NODE_WIDTH, node.h];
// };

// export const getSwitchNodeChildren = (node: GameEventChildSwitch): string[] => {
//   const children: string[] = [];
//   // Add all case next IDs
//   node.cases.forEach((caseItem) => {
//     if (caseItem.next) {
//       children.push(caseItem.next);
//     }
//   });
//   // Add default next if it exists
//   if (node.defaultNext) {
//     children.push(node.defaultNext);
//   }
//   return children;
// };

// export const getSwitchNodeMinHeight = () => {
//   const NODE_TITLE_HEIGHT = 20;
//   const BORDER_WIDTH = 2;
//   const PADDING = 6;
//   const minHeight = NODE_TITLE_HEIGHT + PADDING * 2 + BORDER_WIDTH * 2;
//   return minHeight;
// }

// export const getSwitchNodeLineHeight = () => {
//   return LINE_HEIGHT;
// }

// export const renderSwitchNode = (
//   node: GameEventChildSwitch,
//   x: number,
//   y: number,
//   scale: number,
//   ctx: CanvasRenderingContext2D,
//   args: RenderNodeArgs
// ) => {
//   // Calculate height based on number of cases
//   const minHeight = NODE_TITLE_HEIGHT + PADDING * 2 + BORDER_WIDTH * 2;
//   const totalCaseHeight = (node.cases.length + 1) * LINE_HEIGHT;
//   node.h = Math.max(minHeight, totalCaseHeight + minHeight);

//   // const switchNode = node as GameEventChildSwitch;
//   // const yOffset = 24;
//   // // For switch nodes, we need multiple exits - one for each case plus default
//   // const totalExits = switchNode.cases.length + 1;
//   // const exitSpacing = (node.h - yOffset) / (totalExits + 1);

//   // // Add exits for each case
//   // for (let i = 0; i < switchNode.cases.length; i++) {
//   //   baseCoordinates.exits.push({
//   //     x: node.x + nodeWidth,
//   //     y: node.y + yOffset + exitSpacing * (i + 1),
//   //   });
//   // }

//   // baseCoordinates.exits.push({
//   //   x: node.x + nodeWidth,
//   //   y: node.y + yOffset + exitSpacing * (switchNode.cases.length + 1),
//   // });

//   const nodeX = x * scale;
//   const nodeY = y * scale;
//   const nodeWidth = NODE_WIDTH * scale;
//   const nodeHeight = node.h * scale;
//   const { exits } = getAnchorCoordinates(node);

//   // Draw border - red if selected, white if hovered, light green if child of hovered, light red if parent of hovered, gray otherwise
//   const borderColor = args.isSelected
//     ? '#ff0000'
//     : args.isHovered
//     ? BORDER_HOVER_COLOR
//     : args.isChildOfHovered
//     ? '#90EE90' // Light green
//     : args.isParentOfHovered
//     ? '#FFB6C1' // Light red
//     : BORDER_COLOR;
//   const borderWidth = args.isSelected ? 3 : BORDER_WIDTH;

//   drawRect(nodeX, nodeY, nodeWidth, nodeHeight, borderColor, false, ctx);

//   // Draw additional border for selected nodes
//   if (args.isSelected) {
//     drawRect(
//       nodeX + borderWidth,
//       nodeY + borderWidth,
//       nodeWidth - borderWidth * 2,
//       nodeHeight - borderWidth * 2,
//       borderColor,
//       false,
//       ctx
//     );
//   }
//   drawRect(
//     nodeX + BORDER_WIDTH,
//     nodeY + BORDER_WIDTH,
//     nodeWidth - BORDER_WIDTH * 2,
//     nodeHeight - BORDER_WIDTH * 2,
//     NODE_COLOR,
//     false,
//     ctx
//   );

//   renderCloseButton(node, scale, ctx, {
//     isHovered: args.isCloseButtonHovered || false,
//     isActive: false,
//   });

//   // title
//   ctx.save();
//   ctx.translate(nodeX, nodeY);
//   ctx.scale(scale, scale);
//   ctx.translate(PADDING + BORDER_WIDTH, PADDING + BORDER_WIDTH);
//   drawText(
//     node.id,
//     0,
//     0,
//     {
//       color: '#ddd',
//       size: FONT_SIZE,
//       font: 'arial',
//       strokeColor: '',
//       align: 'left',
//       baseline: 'top',
//     },
//     ctx
//   );
//   ctx.restore();

//   const caseTextOffset = 9;
//   // Render each case's conditionStr
//   for (let i = 0; i < node.cases.length; i++) {
//     const caseItem = node.cases[i];
//     ctx.save();
//     ctx.translate(nodeX, nodeY);
//     ctx.scale(scale, scale);
//     ctx.translate(
//       PADDING + BORDER_WIDTH,
//       caseTextOffset +
//         PADDING +
//         BORDER_WIDTH +
//         NODE_TITLE_HEIGHT +
//         i * LINE_HEIGHT
//     );
//     drawText(
//       caseItem.conditionStr || '',
//       0,
//       0,
//       {
//         color: TEXT_COLOR,
//         size: FONT_SIZE,
//         font: FONT_FAMILY,
//         strokeColor: '',
//         align: 'left',
//       },
//       ctx
//     );
//     ctx.lineWidth = 1;
//     ctx.strokeStyle = 'rgba(255, 255, 255, 0.4)';
//     ctx.beginPath();
//     ctx.moveTo(0, caseTextOffset - 2);
//     ctx.lineTo(NODE_WIDTH - PADDING * 2 - BORDER_WIDTH * 2, caseTextOffset - 2);
//     ctx.stroke();
//     ctx.restore();
//   }

//   // render line for default
//   ctx.save();
//   ctx.translate(nodeX, nodeY);
//   ctx.scale(scale, scale);
//   ctx.translate(
//     PADDING + BORDER_WIDTH,
//     caseTextOffset +
//       PADDING +
//       BORDER_WIDTH +
//       NODE_TITLE_HEIGHT +
//       node.cases.length * LINE_HEIGHT
//   );
//   drawText(
//     'default',
//     0,
//     0,
//     {
//       color: TEXT_COLOR,
//       size: FONT_SIZE,
//       font: FONT_FAMILY,
//       strokeColor: '',
//       align: 'left',
//     },
//     ctx
//   );
//   ctx.lineWidth = 1;
//   ctx.strokeStyle = 'rgba(255, 255, 255, 0.4)';
//   ctx.beginPath();
//   ctx.moveTo(0, caseTextOffset - 2);
//   ctx.lineTo(NODE_WIDTH - PADDING * 2 - BORDER_WIDTH * 2, caseTextOffset - 2);
//   ctx.stroke();
//   ctx.restore();

//   // Render exit anchors as partially transparent red circles

//   const EXIT_ANCHOR_RADIUS = 10; // 10px radius
//   const HOVERED_EXIT_ANCHOR_RADIUS = 14; // 14px radius when hovered

//   for (let i = 0; i < exits.length; i++) {
//     const exit = exits[i];
//     const isHovered = args.hoveredExitIndex === i;
//     const isLinking = args.linkingExitIndex === i;
//     const radius = isHovered ? HOVERED_EXIT_ANCHOR_RADIUS : EXIT_ANCHOR_RADIUS;
    
//     // Check if this exit has a connected node
//     let isConnected = false;
//     if (i < node.cases.length) {
//       // Check case next
//       isConnected = !!(node.cases[i].next && node.cases[i].next !== '');
//     } else if (i === node.cases.length) {
//       // Check default next
//       isConnected = !!(node.defaultNext && node.defaultNext !== '');
//     }
    
//     ctx.save();
//     // Exit coordinates are in world space, convert to canvas space
//     const exitX = exit.x * scale;
//     const exitY = exit.y * scale;

//     // Draw circle - white when linking, brighter when hovered, green if connected, red otherwise
//     if (isLinking) {
//       ctx.fillStyle = 'rgba(255, 255, 255, 0.9)'; // White when linking
//     } else if (isHovered) {
//       if (isConnected) {
//         ctx.fillStyle = 'rgba(100, 255, 100, 0.8)'; // Brighter green when hovered and connected
//       } else {
//         ctx.fillStyle = 'rgba(255, 100, 100, 0.8)'; // Brighter red when hovered and not connected
//       }
//     } else {
//       if (isConnected) {
//         ctx.fillStyle = 'rgba(0, 255, 0, 0.5)'; // Green when connected
//       } else {
//         ctx.fillStyle = 'rgba(255, 0, 0, 0.5)'; // Red when not connected
//       }
//     }
//     ctx.beginPath();
//     ctx.arc(exitX, exitY, radius * scale, 0, Math.PI * 2);
//     ctx.fill();
//     drawText(i.toString(), exitX, exitY, {
//       color: 'blue',
//       size: FONT_SIZE,
//       font: FONT_FAMILY,
//       strokeColor: '',
//       align: 'left',
//     }, ctx);
//     ctx.restore();
//   }
// };
