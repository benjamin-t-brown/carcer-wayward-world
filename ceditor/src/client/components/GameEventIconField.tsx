import { GameEvent } from '../types/assets';
import { SpritePicker } from '../elements/SpritePicker';
import { isGameEventIconRequired } from '../utils/talkEventPortrait';

interface GameEventIconFieldProps {
  id: string;
  eventType: GameEvent['eventType'];
  value: string;
  onChange: (value: string) => void;
}

export function GameEventIconField({
  id,
  eventType,
  value,
  onChange,
}: GameEventIconFieldProps) {
  const required = isGameEventIconRequired(eventType);
  return (
    <div className="form-group" style={{ marginTop: '15px' }}>
      <label htmlFor={id}>{required ? 'Icon *' : 'Default icon'}</label>
      {!required ? (
        <div style={{ marginTop: '4px', fontSize: '12px', color: '#858585' }}>
          Talk portraits use the character's portrait first. This icon is
          the fallback.
        </div>
      ) : null}
      <div style={{ marginTop: '8px' }}>
        <SpritePicker value={value} onChange={onChange} scale={2} />
      </div>
      {value ? (
        <div style={{ marginTop: '8px', fontSize: '12px', color: '#858585' }}>
          Selected: {value}
        </div>
      ) : null}
    </div>
  );
}
