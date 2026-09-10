import type {
  AnimationDefinition,
  MediaCatalog,
  MediaSourceFiles,
  ParsedAssetDefinitions,
  SoundFileMode,
} from './types.js';

export function resolveSoundPath(
  path: string,
  mode: SoundFileMode = 'wav',
): string {
  const extension = `.${mode}`;
  const slash = Math.max(path.lastIndexOf('/'), path.lastIndexOf('\\'));
  const basenameStart = slash + 1;
  const dot = path.lastIndexOf('.');
  return dot > basenameStart
    ? `${path.slice(0, dot)}${extension}`
    : `${path}${extension}`;
}

export function parseSoundVolume(token: string | undefined): number | null {
  if (token === undefined || !/^-?\d+(?:\.\d+)?$/.test(token)) {
    return null;
  }

  const value = Number(token);
  if (!Number.isFinite(value)) {
    return null;
  }
  return Math.min(1, Math.max(0, value));
}

export function parseAssetDefinitions(
  content: string,
  soundMode: SoundFileMode = 'wav',
): ParsedAssetDefinitions {
  const parsed: ParsedAssetDefinitions = {
    sprites: [],
    animations: [],
    sounds: [],
    pictures: {},
  };
  const nextSpriteIndex: Record<string, number> = {};
  let currentAnimation: AnimationDefinition | undefined;

  const finishAnimation = (): void => {
    if (currentAnimation) {
      parsed.animations.push(currentAnimation);
      currentAnimation = undefined;
    }
  };

  for (const sourceLine of content.split(/\r?\n/)) {
    const line = sourceLine.trim();
    if (line === '' || line.startsWith('#')) {
      continue;
    }

    const commaParts = line.split(',').map((part) => part.trim());
    const command = commaParts[0];

    if (command === 'Pic' && commaParts.length >= 3) {
      const alias = commaParts[1];
      const path = commaParts[2];
      if (alias && path) {
        parsed.pictures[alias] = path;
        nextSpriteIndex[alias] ??= 0;
      }
      continue;
    }

    if (command === 'Sound' && commaParts.length >= 3) {
      const name = commaParts[1];
      const path = commaParts[2];
      if (name && path) {
        parsed.sounds.push({
          name,
          path: resolveSoundPath(path, soundMode),
          volume: parseSoundVolume(commaParts[3]) ?? 1,
        });
      }
      continue;
    }

    if (command === 'Sprites' && commaParts.length >= 5) {
      const pictureAlias = commaParts[1];
      const count = Number.parseInt(commaParts[2] ?? '', 10);
      const width = Number.parseInt(commaParts[3] ?? '', 10);
      const height = Number.parseInt(commaParts[4] ?? '', 10);
      const picturePath = pictureAlias
        ? parsed.pictures[pictureAlias]
        : undefined;

      if (
        pictureAlias &&
        picturePath &&
        Number.isInteger(count) &&
        count >= 0 &&
        Number.isInteger(width) &&
        width > 0 &&
        Number.isInteger(height) &&
        height > 0
      ) {
        const startIndex = nextSpriteIndex[pictureAlias] ?? 0;
        for (let offset = 0; offset < count; offset += 1) {
          parsed.sprites.push({
            name: `${pictureAlias}_${offset}`,
            pictureAlias,
            picturePath,
            index: startIndex + offset,
            width,
            height,
          });
        }
        nextSpriteIndex[pictureAlias] = startIndex + count;
      }
      continue;
    }

    if (command === 'Anim' && commaParts.length >= 3) {
      finishAnimation();
      const name = commaParts[1];
      if (name) {
        currentAnimation = {
          name,
          loop: commaParts[2] === 'loop',
          frames: [],
        };
      }
      continue;
    }

    if (line === 'EndAnim') {
      finishAnimation();
      continue;
    }

    if (currentAnimation) {
      const frameParts = line.includes(' ') ? line.split(/\s+/) : commaParts;
      const spriteName = frameParts[0];
      const frames = Number.parseInt(frameParts[1] ?? '', 10);
      if (spriteName && Number.isInteger(frames)) {
        currentAnimation.frames.push({ spriteName, frames });
      }
    }
  }

  finishAnimation();
  return parsed;
}

export function createMediaCatalog(
  sources: Readonly<MediaSourceFiles>,
  soundMode: SoundFileMode = 'wav',
): MediaCatalog {
  const sprites = [];
  const animations = [];
  const sounds = [];
  const pictures: Record<string, string> = {};

  for (const filename of Object.keys(sources).sort()) {
    const parsed = parseAssetDefinitions(sources[filename] ?? '', soundMode);
    sprites.push(...parsed.sprites);
    animations.push(...parsed.animations);
    sounds.push(...parsed.sounds);
    Object.assign(pictures, parsed.pictures);
  }

  return {
    sprites,
    animations,
    sounds,
    pictures,
    spriteByName: new Map(sprites.map((sprite) => [sprite.name, sprite])),
    animationByName: new Map(
      animations.map((animation) => [animation.name, animation]),
    ),
    soundByName: new Map(sounds.map((sound) => [sound.name, sound])),
  };
}
