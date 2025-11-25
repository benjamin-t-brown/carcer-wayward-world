export const randomId = () => {
  return Math.random().toString(36).substring(2, 15);
};

export function normalize(
  x: number,
  A: number,
  B: number,
  C: number,
  D: number
): number {
  return C + ((x - A) * (D - C)) / (B - A);
}
