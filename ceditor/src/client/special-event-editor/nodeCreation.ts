// import {
//   GameEventChildChoice,
//   GameEventChildEnd,
//   GameEventChildExec,
//   GameEventChildKeyword,
//   GameEventChildSwitch,
//   GameEventChildType,
// } from '../types/assets';

// export const createRootNode = (): GameEventChildExec => {
//   return {
//     id: 'root',
//     x: 0,
//     y: 0,
//     h: 0,
//     eventChildType: GameEventChildType.EXEC,
//     p: '',
//     execStr: '',
//     next: '',
//   };
// };

// export const createExecNode = (id: string): GameEventChildExec => {
//   return {
//     id,
//     x: 0,
//     y: 0,
//     h: 0,
//     eventChildType: GameEventChildType.EXEC,
//     p: '',
//     execStr: '',
//     next: '',
//   };
// };

// export const createChoiceNode = (id: string): GameEventChildChoice => {
//   return {
//     id,
//     x: 0,
//     y: 0,
//     h: 0,
//     eventChildType: GameEventChildType.CHOICE,
//     text: '',
//     choices: [],
//   };
// };

// export const createSwitchNode = (id: string): GameEventChildSwitch => {
//   return {
//     id,
//     x: 0,
//     y: 0,
//     h: 0,
//     eventChildType: GameEventChildType.SWITCH,
//     defaultNext: '',
//     cases: [],
//   };
// };

// export const createEndNode = (id: string): GameEventChildEnd => {
//   return {
//     id,
//     x: 0,
//     y: 0,
//     h: 0,
//     eventChildType: GameEventChildType.END,
//     next: '',
//   };
// };

// export const createKeywordNode = (id: string): GameEventChildKeyword => {
//   return {
//     id,
//     x: 0,
//     y: 0,
//     h: 0,
//     eventChildType: GameEventChildType.KEYWORD,
//     keywords: {},
//   };
// };
