import '../../core/ui/base.css';
import '../../core/ui/forms.css';
import './styles.css';
import { mountDatabasePage } from '../../core/ui/index.js';
import { TilesetsEditor } from './view.js';

const app = document.querySelector<HTMLElement>('#app');

if (!app) {
  throw new Error('Tilesets app root not found');
}

mountDatabasePage({
  root: app,
  title: 'Tilesets',
  render(content, context) {
    const editor = new TilesetsEditor(content, context);
    editor.mount();
    return () => editor.destroy();
  },
});
