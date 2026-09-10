import { Suspense, useEffect, useRef, useState } from 'react';
import { readHashRoute } from './utils/hashRoute';
import { Home } from './pages/Home';
import { useAssets } from './contexts/AssetsContext';
import { editorPageForRoute } from './editorPageRoutes';

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

  const EditorPage = editorPageForRoute(routePath);
  if (EditorPage) {
    return (
      <Suspense
        fallback={
          <div className="container">
            <div>Loading editor...</div>
          </div>
        }
      >
        <EditorPage routeParams={routeParams} />
      </Suspense>
    );
  }

  return (
    <div className="container">
      <div className="error">404 - Page not found</div>
    </div>
  );
}

export default App;
