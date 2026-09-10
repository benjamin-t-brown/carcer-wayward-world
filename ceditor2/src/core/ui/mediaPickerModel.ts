import type {
  AnimationDefinition,
  MediaCatalog,
  SoundDefinition,
  SpriteDefinition,
} from '../media/index.js';

export type MediaPickerKind = 'sprite' | 'picture' | 'animation' | 'sound';

export interface PictureChoice {
  readonly name: string;
  readonly path: string;
}

export type MediaPickerChoice =
  SpriteDefinition | AnimationDefinition | SoundDefinition | PictureChoice;

export function mediaChoiceName(choice: MediaPickerChoice): string {
  return choice.name;
}

export function mediaChoices(
  catalog: MediaCatalog,
  kind: MediaPickerKind,
  search = '',
  spriteSheet = '',
): MediaPickerChoice[] {
  const term = search.trim().toLocaleLowerCase();
  const choices: MediaPickerChoice[] =
    kind === 'sprite'
      ? catalog.sprites
      : kind === 'animation'
        ? catalog.animations
        : kind === 'sound'
          ? catalog.sounds
          : Object.entries(catalog.pictures).map(([name, path]) => ({
              name,
              path,
            }));
  return choices
    .filter((choice) => {
      if (
        kind === 'sprite' &&
        spriteSheet &&
        (choice as SpriteDefinition).pictureAlias !== spriteSheet
      ) {
        return false;
      }
      if (!term) return true;
      const values = [mediaChoiceName(choice)];
      if ('path' in choice) values.push(choice.path);
      if ('pictureAlias' in choice) values.push(choice.pictureAlias);
      return values.some((value) => value.toLocaleLowerCase().includes(term));
    })
    .sort((left, right) =>
      mediaChoiceName(left).localeCompare(mediaChoiceName(right)),
    );
}

export function animationSpriteAt(
  animation: AnimationDefinition,
  elapsedMs: number,
  frameDurationMs = 100,
): string | undefined {
  const totalFrames = animation.frames.reduce(
    (sum, frame) => sum + Math.max(1, frame.frames),
    0,
  );
  if (!totalFrames) return undefined;
  const rawFrame = Math.max(0, Math.floor(elapsedMs / frameDurationMs));
  let remaining = animation.loop
    ? rawFrame % totalFrames
    : Math.min(rawFrame, totalFrames - 1);
  for (const frame of animation.frames) {
    const duration = Math.max(1, frame.frames);
    if (remaining < duration) return frame.spriteName;
    remaining -= duration;
  }
  return animation.frames.at(-1)?.spriteName;
}

export function characterSpriteName(
  pictureAlias: string,
  spriteOffset: number | string,
): string {
  return `${pictureAlias}_${spriteOffset}`;
}
