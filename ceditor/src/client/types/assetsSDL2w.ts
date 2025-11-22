// Client-side types matching the server API response
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

