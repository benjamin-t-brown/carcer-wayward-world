// Asset file parsing types
export interface Sprite {
  name: string;
  pictureAlias: string;
  picturePath: string;
  index: number;
  width: number;
  height: number;
}

export interface AnimationFrame {
  spriteName: string;
  frames: number;
}

export interface Animation {
  name: string;
  loop: boolean;
  frames: AnimationFrame[];
}

export interface Sound {
  name: string;
  path: string;
  /** Per-sound volume multiplier in [0, 1]. Defaults to 1. */
  volume: number;
}

export interface ParsedAssetFile {
  sprites: Sprite[];
  animations: Animation[];
  sounds: Sound[];
}

/** Matches sdl2w AssetLoader SoundFileMode. */
export type SoundFileMode = 'wav' | 'ogg';

let soundFileMode: SoundFileMode = 'wav';

export function setSoundFileMode(mode: SoundFileMode) {
  soundFileMode = mode;
}

export function getSoundFileMode(): SoundFileMode {
  return soundFileMode;
}

/**
 * Prefer the configured sound container. Paths with an extension are rewritten;
 * paths without one get the extension appended. Matches sdl2w resolveSoundPath.
 */
export function resolveSoundPath(
  path: string,
  mode: SoundFileMode = soundFileMode
): string {
  const ext = mode === 'ogg' ? '.ogg' : '.wav';
  const slash = Math.max(path.lastIndexOf('/'), path.lastIndexOf('\\'));
  const baseStart = slash >= 0 ? slash + 1 : 0;
  const dot = path.lastIndexOf('.');

  if (dot > baseStart) {
    return path.slice(0, dot) + ext;
  }
  return path + ext;
}

/**
 * Optional Sound line field after path: volume in [0, 1].
 * Non-numeric tokens (attributions) are ignored.
 */
export function tryParseSoundVolume(token: string): number | null {
  if (!/^-?\d+(\.\d+)?$/.test(token)) {
    return null;
  }
  const value = Number(token);
  if (!Number.isFinite(value)) {
    return null;
  }
  return Math.min(1, Math.max(0, value));
}

// Parse asset file
export async function parseAssetFile(content: string) {
  const lines = content.split('\n');

  const sprites: Sprite[] = [];
  const animations: Animation[] = [];
  const sounds: Sound[] = [];
  const pictures: Record<string, string> = {};

  // Track all pictures by alias
  let nextSpriteIndex: Record<string, number> = {};
  let currentAnimation: Animation | null = null;

  for (const line of lines) {
    const trimmed = line.trim();

    // Skip empty lines and comments
    if (!trimmed || trimmed.startsWith('#')) {
      continue;
    }

    // Split by comma for commands
    const parts = trimmed.split(',').map((p) => p.trim());

    if (parts[0] === 'Pic' && parts.length >= 3) {
      const alias = parts[1];
      const path = parts[2];
      pictures[alias] = path;
      if (!(alias in nextSpriteIndex)) {
        nextSpriteIndex[alias] = 0;
      }
    } else if (parts[0] === 'Sound' && parts.length >= 3) {
      // Sound,<alias>,<path>[,<volume 0-1>][,attribution...]
      let volume = 1;
      if (parts.length >= 4) {
        const parsedVolume = tryParseSoundVolume(parts[3]);
        if (parsedVolume !== null) {
          volume = parsedVolume;
        }
      }
      sounds.push({
        name: parts[1],
        path: resolveSoundPath(parts[2]),
        volume,
      });
    } else if (parts[0] === 'Sprites' && parts.length >= 5) {
      const picName = parts[1];
      const numSprites = parseInt(parts[2], 10);
      const spriteWidth = parseInt(parts[3], 10);
      const spriteHeight = parseInt(parts[4], 10);

      if (!isNaN(numSprites) && !isNaN(spriteWidth) && !isNaN(spriteHeight)) {
        // Find the picture for this sprite set
        const picturePath = pictures[picName];
        if (!picturePath) {
          // Picture not found, but continue anyway (C++ code warns but continues)
          continue;
        }

        // Initialize if not already set
        if (!(picName in nextSpriteIndex)) {
          nextSpriteIndex[picName] = 0;
        }

        const startIndex = nextSpriteIndex[picName];

        for (let i = 0; i < numSprites; i++) {
          const spriteIndex = startIndex + i;
          sprites.push({
            name: `${picName}_${i}`, // Sprite name uses counter from 0, not the index
            pictureAlias: picName,
            picturePath: picturePath,
            index: spriteIndex,
            width: spriteWidth,
            height: spriteHeight,
          });
        }

        nextSpriteIndex[picName] = startIndex + numSprites;
      }
    } else if (parts[0] === 'Anim' && parts.length >= 3) {
      // Save previous animation if exists
      if (currentAnimation) {
        animations.push(currentAnimation);
      }

      const animName = parts[1];
      const loop = parts[2] === 'loop';
      currentAnimation = {
        name: animName,
        loop,
        frames: [],
      };
    } else if (trimmed === 'EndAnim') {
      // End current animation
      if (currentAnimation) {
        animations.push(currentAnimation);
        currentAnimation = null;
      }
    } else if (currentAnimation) {
      // Frame line: "spriteName frames" or "spriteName,frames"
      let spriteName: string | undefined;
      let frameCount: number | undefined;

      if (parts.length >= 2 && !trimmed.includes(' ')) {
        spriteName = parts[0];
        frameCount = parseInt(parts[1], 10);
      } else {
        const frameParts = trimmed.split(/\s+/);
        if (frameParts.length >= 2) {
          spriteName = frameParts[0];
          frameCount = parseInt(frameParts[1], 10);
        }
      }

      if (spriteName && frameCount !== undefined && !isNaN(frameCount)) {
        currentAnimation.frames.push({
          spriteName,
          frames: frameCount,
        });
      }
    }
  }

  // Don't forget the last animation if file doesn't end with EndAnim
  if (currentAnimation) {
    animations.push(currentAnimation);
  }

  return { sprites, animations, sounds, pictures };
}

// Load sprites, animations, and sounds from SDL2W asset definition files
export async function loadSpritesAndAnimations(
  assetFilesContent: Record<string, string>
) {
  const allSprites: Sprite[] = [];
  const allAnimations: Animation[] = [];
  const allSounds: Sound[] = [];
  const allPictures: Record<string, string> = {};
  for (const [file, content] of Object.entries(assetFilesContent)) {
    console.log(`Parsing asset file: ${file}`);
    const parsed = await parseAssetFile(content);
    allSprites.push(...parsed.sprites);
    allAnimations.push(...parsed.animations);
    allSounds.push(...parsed.sounds);
    Object.assign(allPictures, parsed.pictures);
  }

  return {
    sprites: allSprites,
    animations: allAnimations,
    sounds: allSounds,
    pictures: allPictures,
  };
}
