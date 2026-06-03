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
}

export interface ParsedAssetFile {
  sprites: Sprite[];
  animations: Animation[];
  sounds: Sound[];
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
      sounds.push({
        name: parts[1],
        path: parts[2],
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
