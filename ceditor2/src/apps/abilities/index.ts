import '../../core/ui/base.css';
import '../../core/ui/forms.css';

const app = document.querySelector<HTMLElement>('#app');

if (!app) {
  throw new Error('Abilities app root not found');
}

app.innerHTML =
  '<h1>Abilities</h1><p>The abilities editor is ready to be built.</p>';
