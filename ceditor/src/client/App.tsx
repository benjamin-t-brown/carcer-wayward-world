import { useEffect, useState } from 'react';
import { Home } from './pages/Home';
import { ItemTemplates } from './pages/ItemTemplates';
import { CharacterTemplates } from './pages/CharacterTemplates';
import { TilesetTemplates } from './pages/TilesetTemplates';
import { SpecialEvents } from './pages/SpecialEvents';
import { Maps } from './pages/Maps';

function App({ assetTypes }: { assetTypes: { id: string; name: string; file: string }[] }) {
  const [currentRoute, setCurrentRoute] = useState<string>('/');

  useEffect(() => {
    // Handle hash-based routing
    const handleRoute = () => {
      const hash = window.location.hash.slice(1) || '/';
      setCurrentRoute(hash);
    };

    window.addEventListener('hashchange', handleRoute);
    handleRoute(); // Initial route

    return () => {
      window.removeEventListener('hashchange', handleRoute);
    };
  }, []);

  // Route rendering
  switch (currentRoute) {
    case '/':
      return <Home assetTypes={assetTypes} />;
    case '/editor/itemTemplates':
      return <ItemTemplates />;
    case '/editor/characterTemplates':
      return <CharacterTemplates />;
    case '/editor/tilesetTemplates':
      return <TilesetTemplates />;
    case '/editor/specialEvents':
      return <SpecialEvents />;
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

