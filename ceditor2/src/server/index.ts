import { createApp } from './app.js';
import { resolveServerPort, SERVER_HOST } from './config.js';

const port = resolveServerPort();
const app = createApp();

app.listen(port, SERVER_HOST, () => {
  console.log(`CEditor2 server listening at http://${SERVER_HOST}:${port}`);
});
