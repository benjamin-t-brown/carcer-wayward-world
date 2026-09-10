import '../../core/ui/base.css';
import '../../core/ui/forms.css';

const app = document.querySelector<HTMLElement>('#app');

if (!app) {
  throw new Error('Map grids app root not found');
}

app.innerHTML =
  '<h1>Map Grids</h1><p>The map-grids editor is ready to be built.</p>';
