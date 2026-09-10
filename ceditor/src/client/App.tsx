import { useEffect, useRef, useState } from 'react';
import { readHashRoute } from './utils/hashRoute';
import { Home } from './pages/Home';
import { ItemTemplates } from './pages/ItemTemplates';
import { CharacterTemplates } from './pages/CharacterTemplates';
import { TilesetTemplates } from './pages/TilesetTemplates';
import { SpecialEvents } from './pages/SpecialEvents';
import { AbilityTemplates } from './pages/AbilityTemplates';
import { SpellTemplates } from './pages/SpellTemplates';
import { StatusEffectTemplates } from './pages/StatusEffectTemplates';
import { Maps } from './pages/Maps';
import { MapGrids } from './pages/MapGrids';
import { SoundEffects } from './pages/SoundEffects';
import { assetIdForEditorRoute } from './utils/editorRoutes';
import { useAssets } from './contexts/AssetsContext';

function App({
  assetTypes,
}: {
  assetTypes: { id: string; name: string; file: string }[];
}) {
  const { isDirty } = useAssets();
  const initialRoute = readHashRoute();
  const [currentRoute, setCurrentRoute] = useState<string>(initialRoute.path);
  const [routeParams, setRouteParams] = useState<URLSearchParams>(
    initialRoute.params,
  );
  const acceptedHashRef = useRef(window.location.hash || '#/');

  useEffect(() => {
    const handleRoute = () => {
      const { path, params } = readHashRoute();
      if (
        path !== currentRoute &&
        isDirty &&
        !window.confirm('You have unsaved database changes. Leave this editor?')
      ) {
        window.history.replaceState(null, '', acceptedHashRef.current);
        return;
      }

      acceptedHashRef.current = window.location.hash || '#/';
      setCurrentRoute(path);
      setRouteParams(params);
    };

    window.addEventListener('hashchange', handleRoute);
    handleRoute();

    return () => {
      window.removeEventListener('hashchange', handleRoute);
    };
  }, [currentRoute, isDirty]);

  // Route rendering
  const routePath = currentRoute.split('?')[0]; // Ensure we only match on the path part
  if (routePath === '/') {
    return <Home assetTypes={assetTypes} />;
  }

  switch (assetIdForEditorRoute(routePath)) {
    case 'itemTemplates':
      return <ItemTemplates routeParams={routeParams} />;
    case 'abilityTemplates':
      return <AbilityTemplates routeParams={routeParams} />;
    case 'spellTemplates':
      return <SpellTemplates routeParams={routeParams} />;
    case 'statusEffectTemplates':
      return <StatusEffectTemplates routeParams={routeParams} />;
    case 'characterTemplates':
      return <CharacterTemplates routeParams={routeParams} />;
    case 'tilesetTemplates':
      return <TilesetTemplates routeParams={routeParams} />;
    case 'specialEvents':
      return <SpecialEvents routeParams={routeParams} />;
    case 'maps':
      return <Maps routeParams={routeParams} />;
    case 'mapGrids':
      return <MapGrids routeParams={routeParams} />;
    default:
      if (routePath === '/editor/soundEffects') {
        return <SoundEffects />;
      }
      return (
        <div className="container">
          <div className="error">404 - Page not found</div>
        </div>
      );
  }
}

export default App;
