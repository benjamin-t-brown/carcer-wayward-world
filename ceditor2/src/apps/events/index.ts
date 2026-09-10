import '../../core/ui/base.css';
import './styles.css';

const app = document.querySelector<HTMLElement>('#app');

if (!app) {
  throw new Error('Special events app root not found');
}

app.innerHTML = `
  <section class="events-placeholder">
    <h1>Special Events</h1>
    <p>The canvas event editor is ready to be built.</p>
  </section>
`;
