import '../../core/ui/base.css';
import '../../core/ui/forms.css';

const app = document.querySelector<HTMLElement>('#app');

if (!app) {
  throw new Error('Tilesets app root not found');
}

app.innerHTML =
  '<h1>Tilesets</h1><p>The tilesets editor is ready to be built.</p>';
