import type { SoundDefinition } from '../../core/media/index.js';

export function matchesSoundSearch(
  sound: SoundDefinition,
  searchTerm: string,
): boolean {
  const term = searchTerm.trim().toLocaleLowerCase();
  return (
    !term ||
    sound.name.toLocaleLowerCase().includes(term) ||
    sound.path.toLocaleLowerCase().includes(term)
  );
}
