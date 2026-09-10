import '../../core/ui/base.css';
import '../../core/ui/forms.css';

const app = document.querySelector<HTMLElement>('#app');

if (!app) {
  throw new Error('Characters app root not found');
}

app.innerHTML =
  '<h1>Characters</h1><p>The characters editor is ready to be built.</p>';
