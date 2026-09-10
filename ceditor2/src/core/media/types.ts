export interface SpriteDefinition {
  name: string;
  pictureAlias: string;
  picturePath: string;
  index: number;
  width: number;
  height: number;
}

export interface AnimationFrameDefinition {
  spriteName: string;
  frames: number;
}

export interface AnimationDefinition {
  name: string;
  loop: boolean;
  frames: AnimationFrameDefinition[];
}

export interface SoundDefinition {
  name: string;
  path: string;
  volume: number;
}

export interface ParsedAssetDefinitions {
  sprites: SpriteDefinition[];
  animations: AnimationDefinition[];
  sounds: SoundDefinition[];
  pictures: Record<string, string>;
}

export interface MediaCatalog extends ParsedAssetDefinitions {
  spriteByName: ReadonlyMap<string, SpriteDefinition>;
  animationByName: ReadonlyMap<string, AnimationDefinition>;
  soundByName: ReadonlyMap<string, SoundDefinition>;
}

export type MediaSourceFiles = Record<string, string>;

export type SoundFileMode = 'wav' | 'ogg';
