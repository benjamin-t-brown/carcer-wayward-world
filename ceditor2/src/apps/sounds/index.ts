import '../../core/ui/base.css';
import '../../core/ui/forms.css';
import { mountDatabasePage } from '../../core/ui/index.js';
import { SoundBrowser } from './view.js';
import './styles.css';

const app = document.querySelector<HTMLElement>('#app');
if (!app) throw new Error('Sounds app root not found');

mountDatabasePage({
  root: app,
  title: 'Sounds',
  render(content) {
    const browser = new SoundBrowser(content);
    browser.mount();
    return () => browser.destroy();
  },
});
