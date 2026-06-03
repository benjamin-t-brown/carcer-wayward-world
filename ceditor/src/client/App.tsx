import { useEffect, useState } from 'react';
import { readHashRoute } from './utils/hashRoute';
import { Home } from './pages/Home';
import { ItemTemplates } from './pages/ItemTemplates';
import { CharacterTemplates } from './pages/CharacterTemplates';
import { TilesetTemplates } from './pages/TilesetTemplates';
import { SpecialEvents } from './pages/SpecialEvents';
import { AbilityTemplates } from './pages/AbilityTemplates';
import { StatusEffectTemplates } from './pages/StatusEffectTemplates';
import { Maps } from './pages/Maps';

function App({ assetTypes }: { assetTypes: { id: string; name: string; file: string }[] }) {
  const initialRoute = readHashRoute();
  const [currentRoute, setCurrentRoute] = useState<string>(initialRoute.path);
  const [routeParams, setRouteParams] = useState<URLSearchParams>(
    initialRoute.params
  );

  useEffect(() => {
    const handleRoute = () => {
      const { path, params } = readHashRoute();
      setCurrentRoute(path);
      setRouteParams(params);
    };

    window.addEventListener('hashchange', handleRoute);
    handleRoute();

    return () => {
      window.removeEventListener('hashchange', handleRoute);
    };
  }, []);

  // Route rendering
  const routePath = currentRoute.split('?')[0]; // Ensure we only match on the path part
  switch (routePath) {
    case '/':
      return <Home assetTypes={assetTypes} />;
    case '/editor/itemTemplates':
      return <ItemTemplates routeParams={routeParams} />;
    case '/editor/abilityTemplates':
      return <AbilityTemplates routeParams={routeParams} />;
    case '/editor/statusEffectTemplates':
      return <StatusEffectTemplates />;
    case '/editor/characterTemplates':
      return <CharacterTemplates routeParams={routeParams} />;
    case '/editor/tilesetTemplates':
      return <TilesetTemplates routeParams={routeParams} />;
    case '/editor/specialEvents':
      return <SpecialEvents routeParams={routeParams} />;
    case '/editor/maps':
      return <Maps />;
    default:
      return (
        <div className="container">
          <div className="error">404 - Page not found</div>
        </div>
      );
  }
}

export default App;

