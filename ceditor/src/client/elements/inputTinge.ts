export const INPUT_TINGES = [
  'red',
  'orange',
  'yellow',
  'green',
  'blue',
  'indigo',
  'violet',
] as const;

export type InputTinge = (typeof INPUT_TINGES)[number];

export function tingeClassName(tinge?: InputTinge): string {
  return tinge ? `tinged tinged-${tinge}` : '';
}
