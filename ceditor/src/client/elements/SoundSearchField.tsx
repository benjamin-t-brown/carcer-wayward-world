import { useMemo } from 'react';
import { SearchSelect } from './SearchSelect';
import { SoundPreview } from './SoundPreview';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';

interface SoundSearchFieldProps {
  id: string;
  name: string;
  label: string;
  value: string;
  onChange: (value: string) => void;
  className?: string;
  placeholder?: string;
  emptyLabel?: string;
}

export function SoundSearchField({
  id,
  name,
  label,
  value,
  onChange,
  className = 'sound-search-field',
  placeholder = 'Search sounds...',
  emptyLabel = 'None',
}: SoundSearchFieldProps) {
  const { sounds } = useSDL2WAssets();

  const soundItems = useMemo(() => {
    const byName = new Map(sounds.map((sound) => [sound.name, sound]));
    if (value && !byName.has(value)) {
      byName.set(value, { name: value, path: '' });
    }
    return [...byName.values()].sort((a, b) => a.name.localeCompare(b.name));
  }, [sounds, value]);

  return (
    <div className={className}>
      <SoundPreview soundName={value} />
      <SearchSelect
        id={id}
        name={name}
        label={label}
        value={value}
        onChange={onChange}
        items={soundItems}
        getItemKey={(sound) => sound.name}
        getItemLabel={(sound) => sound.name}
        searchFields={(sound) => [sound.name]}
        placeholder={placeholder}
        emptyLabel={emptyLabel}
        allowEmpty
      />
    </div>
  );
}
