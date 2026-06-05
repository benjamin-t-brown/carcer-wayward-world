import { Animation as AnimationPlayer } from '../components/Animation';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';

const PREVIEW_SIZE = 40;

interface AnimationPreviewProps {
  animationName: string;
  displaySize?: number;
}

export function AnimationPreview({
  animationName,
  displaySize = PREVIEW_SIZE,
}: AnimationPreviewProps) {
  const { animationMap, sprites, spriteMap } = useSDL2WAssets();
  const animation = animationName ? animationMap[animationName] : undefined;

  if (!animation) {
    return (
      <div
        className="animation-preview animation-preview-empty"
        style={{ width: displaySize, height: displaySize }}
        aria-hidden="true"
      />
    );
  }

  return (
    <div
      className="animation-preview"
      style={{ width: displaySize, height: displaySize }}
      title={animationName}
    >
      <AnimationPlayer
        key={animationName}
        animation={animation}
        sprites={sprites}
        spriteMap={spriteMap}
        displaySize={displaySize}
      />
    </div>
  );
}
