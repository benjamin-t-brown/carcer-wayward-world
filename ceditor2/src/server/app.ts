import express, {
  type ErrorRequestHandler,
  type RequestHandler,
} from 'express';
import { STATUS_CODES } from 'node:http';
import { fileURLToPath } from 'node:url';

/** Absolute path to the game's existing, read-only asset tree. */
export const GAME_ASSETS_PATH = fileURLToPath(
  new URL('../../../src/assets/', import.meta.url),
);

const notFoundHandler: RequestHandler = (_request, response) => {
  response.status(404).json({ error: 'Not Found' });
};

const errorHandler: ErrorRequestHandler = (
  error: unknown,
  _request,
  response,
  next,
) => {
  if (response.headersSent) {
    next(error);
    return;
  }

  const status = getErrorStatus(error);
  if (status >= 500) {
    console.error('CEditor2 server error:', error);
  }

  response.status(status).json({
    error:
      status === 400
        ? 'Invalid JSON request body'
        : (STATUS_CODES[status] ?? 'Request Failed'),
  });
};

function getErrorStatus(error: unknown): number {
  if (!error || typeof error !== 'object') {
    return 500;
  }

  const candidate = error as { status?: unknown; statusCode?: unknown };
  const status = candidate.status ?? candidate.statusCode;
  return typeof status === 'number' && status >= 400 && status < 600
    ? status
    : 500;
}

export function createApp(): express.Express {
  const app = express();

  app.disable('x-powered-by');
  app.use(express.json({ limit: '50mb' }));

  app.get('/api/health', (_request, response) => {
    response.json({ status: 'ok' });
  });

  app.use(
    '/game-assets',
    express.static(GAME_ASSETS_PATH, {
      dotfiles: 'deny',
      fallthrough: true,
      index: false,
      redirect: false,
    }),
  );

  app.use(notFoundHandler);
  app.use(errorHandler);

  return app;
}
