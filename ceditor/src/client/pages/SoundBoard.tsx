import { EditorHeader } from '../components/EditorHeader';
import { SoundPreview } from '../elements/SoundPreview';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';

export function SoundBoard() {
  const { sounds } = useSDL2WAssets();
  const sorted = sounds.slice().sort((a, b) => a.name.localeCompare(b.name));

  return (
    <div className="container editor-page sound-board-page">
      <EditorHeader title="Sound Board" />

      <div className="editor-page-body sound-board-body">
        {sorted.length === 0 ? (
          <div className="empty-state">No sounds loaded</div>
        ) : (
          <ul className="sound-board-list">
            {sorted.map((sound) => (
              <li key={sound.name} className="sound-board-row">
                <SoundPreview soundName={sound.name} displaySize={28} />
                <span className="sound-board-name">{sound.name}</span>
                <span className="sound-board-volume">
                  {Math.round((sound.volume ?? 1) * 100)}%
                </span>
              </li>
            ))}
          </ul>
        )}
      </div>
    </div>
  );
}
