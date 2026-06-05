import { useEffect, useRef } from 'react';
import { Button } from './Button';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';

const PREVIEW_SIZE = 40;

function getSoundUrl(path: string): string {
  return `/api/${path}`;
}

interface SoundPreviewProps {
  soundName: string;
  displaySize?: number;
}

export function SoundPreview({
  soundName,
  displaySize = PREVIEW_SIZE,
}: SoundPreviewProps) {
  const { soundMap } = useSDL2WAssets();
  const audioRef = useRef<HTMLAudioElement | null>(null);
  const sound = soundName ? soundMap[soundName] : undefined;
  const canPlay = Boolean(sound);

  useEffect(() => {
    return () => {
      if (audioRef.current) {
        audioRef.current.pause();
        audioRef.current = null;
      }
    };
  }, []);

  const handlePlay = () => {
    if (!sound) {
      return;
    }
    if (audioRef.current) {
      audioRef.current.pause();
      audioRef.current.currentTime = 0;
    }
    const audio = new Audio(getSoundUrl(sound.path));
    audioRef.current = audio;
    void audio.play().catch(() => {});
  };

  return (
    <div
      className={`sound-preview ${canPlay ? '' : 'sound-preview-empty'}`.trim()}
      style={{ width: displaySize, height: displaySize }}
      title={canPlay ? soundName : undefined}
    >
      <Button
        type="button"
        variant="small"
        className="btn-card-action sound-preview-play"
        disabled={!canPlay}
        ariaLabel={canPlay ? `Play ${soundName}` : 'No sound selected'}
        onClick={handlePlay}
      >
        ▶
      </Button>
    </div>
  );
}
