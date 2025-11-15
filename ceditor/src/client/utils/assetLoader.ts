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

export interface ParsedAssetFile {
  sprites: Sprite[];
  animations: Animation[];
}

// Parse asset file
export async function parseAssetFile(content: string) {
  const lines = content.split('\n');

  const sprites: Sprite[] = [];
  const animations: Animation[] = [];
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
      // Parse animation frame: "spriteName frames"
      const frameParts = trimmed.split(/\s+/);
      if (frameParts.length >= 2) {
        const spriteName = frameParts[0];
        const frames = parseInt(frameParts[1], 10);
        if (!isNaN(frames)) {
          currentAnimation.frames.push({
            spriteName,
            frames,
          });
        }
      }
    }
  }

  // Don't forget the last animation if file doesn't end with EndAnim
  if (currentAnimation) {
    animations.push(currentAnimation);
  }

  return { sprites, animations, pictures };
}

// Load all sprites and animations from asset files
export async function loadSpritesAndAnimations(
  assetFilesContent: Record<string, string>
) {
  const allSprites: Sprite[] = [];
  const allAnimations: Animation[] = [];
  const allPictures: Record<string, string> = {};
  for (const [file, content] of Object.entries(assetFilesContent)) {
    console.log(`Parsing asset file: ${file}`);
    const parsed = await parseAssetFile(content);
    allSprites.push(...parsed.sprites);
    allAnimations.push(...parsed.animations);
    Object.assign(allPictures, parsed.pictures);
  }

  return {
    sprites: allSprites,
    animations: allAnimations,
    pictures: allPictures,
  };
}
