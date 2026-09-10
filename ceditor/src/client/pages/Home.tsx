import { Card } from '../elements/Card';
import { EditorHeader } from '../components/EditorHeader';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { editorRouteForAssetId } from '../utils/editorRoutes';

export function Home({
  assetTypes,
}: {
  assetTypes: { id: string; name: string; file: string }[];
}) {
  const {
    sprites: _sprites,
    animations: _animations,
    pictures: _pictures,
  } = useSDL2WAssets();
  const handleCardClick = (typeId: string) => {
    const route = editorRouteForAssetId(typeId);
    if (route) {
      window.location.hash = route;
    }
  };

  return (
    <div className="container">
      <EditorHeader title="CEditor" showBack={false} />
      <div className="asset-types">
        {assetTypes.filter((type) => editorRouteForAssetId(type.id)).map((type) => (
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
