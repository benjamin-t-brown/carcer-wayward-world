import { useState, useRef, useEffect, useMemo } from 'react';
import { GameEvent } from '../../../types/assets';

export function EventSearchInput({
  events,
  onSelect,
  placeholder = 'Search events...',
}: {
  events: GameEvent[];
  onSelect: (eventId: string) => void;
  placeholder?: string;
}) {
  const [searchTerm, setSearchTerm] = useState('');
  const [isOpen, setIsOpen] = useState(false);
  const inputRef = useRef<HTMLInputElement>(null);
  const dropdownRef = useRef<HTMLDivElement>(null);

  // Filter to only show MODAL events
  const modalEvents = useMemo(() => {
    return events.filter((event) => event.eventType === 'MODAL');
  }, [events]);

  const filteredEvents = useMemo(() => {
    if (!searchTerm.trim()) return modalEvents;
    const term = searchTerm.toLowerCase();
    return modalEvents.filter(
      (event) =>
        event.id.toLowerCase().includes(term) ||
        event.title.toLowerCase().includes(term)
    );
  }, [modalEvents, searchTerm]);

  useEffect(() => {
    const handleClickOutside = (event: MouseEvent) => {
      if (
        dropdownRef.current &&
        !dropdownRef.current.contains(event.target as Node) &&
        inputRef.current &&
        !inputRef.current.contains(event.target as Node)
      ) {
        setIsOpen(false);
      }
    };

    document.addEventListener('mousedown', handleClickOutside);
    return () => document.removeEventListener('mousedown', handleClickOutside);
  }, []);

  const handleSelect = (eventId: string) => {
    onSelect(eventId);
    setSearchTerm('');
    setIsOpen(false);
  };

  return (
    <div style={{ position: 'relative', width: '100%' }}>
      <input
        ref={inputRef}
        type="text"
        value={searchTerm}
        onChange={(e) => {
          setSearchTerm(e.target.value);
          setIsOpen(true);
        }}
        onFocus={() => setIsOpen(true)}
        placeholder={placeholder}
        style={{
          width: '100%',
          padding: '6px 8px',
          border: '1px solid #3e3e42',
          backgroundColor: '#1e1e1e',
          color: '#ffffff',
          fontSize: '12px',
          borderRadius: '4px',
        }}
      />
      {isOpen && filteredEvents.length > 0 && (
        <div
          ref={dropdownRef}
          style={{
            position: 'absolute',
            top: '100%',
            left: 0,
            right: 0,
            marginTop: '4px',
            maxHeight: '200px',
            overflowY: 'auto',
            backgroundColor: '#252526',
            border: '1px solid #3e3e42',
            borderRadius: '4px',
            zIndex: 1000,
            boxShadow: '0 4px 8px rgba(0, 0, 0, 0.3)',
          }}
        >
          {filteredEvents.map((event) => (
            <div
              key={event.id}
              onClick={() => handleSelect(event.id)}
              style={{
                padding: '8px 12px',
                cursor: 'pointer',
                fontSize: '12px',
                color: '#ffffff',
                borderBottom: '1px solid #3e3e42',
                transition: 'background-color 0.2s',
              }}
              onMouseEnter={(e) => {
                e.currentTarget.style.backgroundColor = '#3e3e42';
              }}
              onMouseLeave={(e) => {
                e.currentTarget.style.backgroundColor = 'transparent';
              }}
            >
              <div style={{ fontWeight: 'bold' }}>{event.title}</div>
              <div
                style={{
                  fontSize: '10px',
                  color: '#858585',
                  marginTop: '2px',
                }}
              >
                {event.id} • {event.eventType}
              </div>
            </div>
          ))}
        </div>
      )}
    </div>
  );
}

