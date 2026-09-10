import '../../core/ui/base.css';
import '../../core/ui/forms.css';
import { mountDatabasePage } from '../../core/ui/index.js';
import { MapGridsEditor } from './view.js';
import './styles.css';

const app = document.querySelector<HTMLElement>('#app');
if (!app) throw new Error('Map grids app root not found');

mountDatabasePage({
  root: app,
  title: 'Map Grids',
  render(content, context) {
    const editor = new MapGridsEditor(content, context);
    editor.mount();
    return () => editor.destroy();
  },
});
