import '../../core/ui/base.css';
import '../../core/ui/forms.css';
import { mountDatabasePage } from '../../core/ui/index.js';
import { MapEditorController } from './MapEditorController.js';
import './styles.css';

const app = document.querySelector<HTMLElement>('#app');

if (!app) {
  throw new Error('Maps app root not found');
}

mountDatabasePage({
  root: app,
  title: 'Maps',
  render(content, context) {
    const editor = new MapEditorController(content, context);
    editor.mount();
    return () => editor.destroy();
  },
});
