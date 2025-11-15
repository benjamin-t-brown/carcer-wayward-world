import { Card } from '../elements/Card';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';

export function Home({
  assetTypes,
}: {
  assetTypes: { id: string; name: string; file: string }[];
}) {
  const { sprites: _sprites, animations: _animations, pictures: _pictures } = useSDL2WAssets();
  const handleCardClick = (typeId: string) => {
    window.location.hash = `/editor/${typeId}`;
  };

  return (
    <div className="container">
      <h1>CEditor</h1>
      <div className="asset-types">
        {assetTypes.map((type) => (
          <Card
            key={type.id}
            variant="asset"
            onClick={() => handleCardClick(type.id)}
          >
            <h2>{type.name}</h2>
            <p>
              Edit {type.name.toLowerCase()} in {type.file}
            </p>
          </Card>
        ))}
      </div>
    </div>
  );
}
