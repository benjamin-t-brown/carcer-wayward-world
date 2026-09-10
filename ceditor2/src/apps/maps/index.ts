import '../../core/ui/base.css';
import './styles.css';

const app = document.querySelector<HTMLElement>('#app');

if (!app) {
  throw new Error('Maps app root not found');
}

app.innerHTML = `
  <section class="maps-placeholder">
    <h1>Maps</h1>
    <p>The canvas map editor is ready to be built.</p>
  </section>
`;
