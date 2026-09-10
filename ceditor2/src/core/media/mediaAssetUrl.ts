/** Convert the paths stored in SDL2W asset files to the editor's static route. */
export function mediaAssetUrl(path: string): string {
  const relativePath = path
    .trim()
    .replace(/^(?:\.\/|\/)+/, '')
    .replace(/^assets\//, '');
  return `/game-assets/${relativePath}`;
}
