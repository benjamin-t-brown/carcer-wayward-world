import { SearchInput } from '../../elements/SearchInput';
import { useReRender } from '../../hooks/useReRender';
import { AudioInfo } from '../../types/assets';
import { EditorNodeChoice } from '../cmpts/ChoiceNodeComponent';
import { EditorNodeExec } from '../cmpts/ExecNodeComponent';

interface AudioWidgetProps {
  node: EditorNodeExec | EditorNodeChoice;
}

export const AudioWidget = ({ node }: AudioWidgetProps) => {
  const reRender = useReRender();
  const audioNames: string[] = [
    'test',
    'test2',
    'test3',
    'test12412412412412dhadhdfahadhdah4',
  ];
  const value = node.audioInfo?.audioName ?? '';
  return (
    <div>
      <div style={{ marginBottom: '4px' }}>Search Audio Names </div>
      <div>
        <SearchInput
          items={audioNames}
          onSelect={(audioName) => {
            if (!node.audioInfo) {
              node.audioInfo = {
                audioName: audioName,
                volume: 100,
                offset: 0,
              };
            } else {
              node.audioInfo.audioName = audioName;
            }
            reRender();
          }}
          searchFields={(audioName: string) => [audioName]}
          renderItem={(audioName: string) => <div>{audioName}</div>}
          getItemKey={(audioName: string) => audioName}
        />
        <div
          style={{
            margin: '4px',
            display: 'inline-block',
            minHeight: '30px',
            width: '100%',
            overflow: 'hidden',
            textOverflow: 'ellipsis',
            whiteSpace: 'pre',
          }}
        >
          {value ? (
            <div
              style={{
                display: 'flex',
                alignItems: 'center',
                gap: '8px',
                overflow: 'hidden',
                textOverflow: 'ellipsis',
                whiteSpace: 'pre',
                width: '100%',
              }}
            >
              <button
                style={{
                  border: 'none',
                  cursor: 'pointer',
                  fontSize: '16px',
                  color: '#ff5555',
                  background: 'black',
                  padding: '2px 6px',
                  borderRadius: '8px'
                }}
                title="Remove Audio"
                onClick={() => {
                  node.audioInfo = undefined;
                  reRender();
                }}
              >
              <span style={{ fontFamily: 'courier new' }}>X</span>
              </button>
              <button
                style={{
                  marginLeft: '0px',
                  border: 'none',
                  cursor: 'pointer',
                  padding: '2px 6px',
                  fontSize: '16px',
                }}
                title="Play Audio"
                onClick={() => {}}
              >
                ▶
              </button>

              <span>{value}</span>
            </div>
          ) : (
            <div>No audio selected</div>
          )}
        </div>
      </div>
    </div>
  );
};
