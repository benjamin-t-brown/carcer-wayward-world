import { useState } from 'react';
import { EditorHeader } from '../components/EditorHeader';
import { SoundPreview } from '../elements/SoundPreview';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';

export function SoundEffects() {
  const { sounds } = useSDL2WAssets();
  const [searchTerm, setSearchTerm] = useState('');

  const term = searchTerm.trim().toLowerCase();
  const filtered = (
    term
      ? sounds.filter(
          (sound) =>
            sound.name.toLowerCase().includes(term) ||
            sound.path.toLowerCase().includes(term)
        )
      : sounds.slice()
  ).sort((a, b) => a.name.localeCompare(b.name));

  return (
    <div className="container editor-page sound-effects-page">
      <EditorHeader title="Sound Effects" />

      <div className="editor-page-body sound-effects-body">
        <div className="sound-effects-panel">
          <div className="search-box">
            <input
              type="text"
              value={searchTerm}
              onChange={(e) => setSearchTerm(e.target.value)}
              placeholder="Search sounds..."
            />
          </div>

          {filtered.length === 0 ? (
            <div className="empty-state">No sounds found</div>
          ) : (
            <ul className="sound-effects-list">
              {filtered.map((sound) => (
                <li key={sound.name} className="sound-effects-row">
                  <span className="sound-effects-name">{sound.name}</span>
                  <span className="sound-effects-path">{sound.path}</span>
                  <SoundPreview soundName={sound.name} displaySize={28} />
                </li>
              ))}
            </ul>
          )}
        </div>
      </div>
    </div>
  );
}
