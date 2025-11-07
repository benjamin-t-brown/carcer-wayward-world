import { router } from './router';
import { renderHome } from './pages/home';
import { renderItemTemplatesEditor } from './pages/itemTemplatesEditor';
import { renderCharacterTemplatesEditor } from './pages/characterTemplatesEditor';

// Register routes
router.register('/', renderHome);
router.register('/editor/itemTemplates', renderItemTemplatesEditor);
router.register('/editor/characterTemplates', renderCharacterTemplatesEditor);

// Handle hash-based routing
function handleRoute() {
  const hash = window.location.hash.slice(1) || '/';
  router.navigate(hash);
}

window.addEventListener('hashchange', handleRoute);

// Initialize router
handleRoute();

