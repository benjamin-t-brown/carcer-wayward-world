import { SearchSelect } from '../elements/SearchSelect';
import { useAssets } from '../contexts/AssetsContext';

interface ItemUseSpecialEventFieldsProps {
  useSpecialEvent: string | undefined;
  onChange: (useSpecialEvent: string | undefined) => void;
  idPrefix: string;
}

export function ItemUseSpecialEventFields({
  useSpecialEvent,
  onChange,
  idPrefix,
}: ItemUseSpecialEventFieldsProps) {
  const { gameEvents } = useAssets();

  const handleSpecialEventChange = (eventId: string) => {
    onChange(eventId || undefined);
  };

  return (
    <div className="form-subsection item-use-ability-fields">
      <div className="weapon-ability-row">
        <SearchSelect
          id={`${idPrefix}-special-event`}
          name="useSpecialEvent"
          label="Use Special Event"
          value={useSpecialEvent ?? ''}
          onChange={handleSpecialEventChange}
          items={gameEvents}
          getItemKey={(event) => event.id}
          getItemLabel={(event) => event.title?.trim() || event.id}
          searchFields={(event) => [
            event.id,
            event.title ?? '',
            event.eventType,
          ]}
          placeholder="Search special events..."
          emptyLabel="(select special event)"
          renderItem={(event) => (
            <>
              <div style={{ fontWeight: 600 }}>
                {event.title?.trim() || event.id}
              </div>
              <div
                style={{ fontSize: '10px', color: '#858585', marginTop: '2px' }}
              >
                {event.id} • {event.eventType}
              </div>
            </>
          )}
        />
        {useSpecialEvent ? (
          <a
            className="template-edit-link"
            href={`${window.location.origin}${window.location.pathname}#/editor/specialEvents?event=${encodeURIComponent(useSpecialEvent)}`}
            target="_blank"
            rel="noopener noreferrer"
          >
            Edit special event
          </a>
        ) : null}
      </div>
    </div>
  );
}
