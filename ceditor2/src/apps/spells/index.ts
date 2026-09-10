import '../../core/ui/base.css';
import '../../core/ui/forms.css';

const app = document.querySelector<HTMLElement>('#app');

if (!app) {
  throw new Error('Spells app root not found');
}

app.innerHTML = '<h1>Spells</h1><p>The spells editor is ready to be built.</p>';
