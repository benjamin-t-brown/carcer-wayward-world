import express, {
  type ErrorRequestHandler,
  type RequestHandler,
} from 'express';
import { STATUS_CODES } from 'node:http';
import { join } from 'node:path';
import { fileURLToPath } from 'node:url';

import {
  DatabaseRepository,
  type DatabaseRepositoryContract,
} from './databaseRepository.js';
import { isHttpError } from './httpErrors.js';
import {
  MediaSourceRepository,
  type MediaSourceRepositoryContract,
} from './mediaRepository.js';

/** Absolute path to the game's existing, read-only asset tree. */
export const GAME_ASSETS_PATH = fileURLToPath(
  new URL('../../../src/assets/', import.meta.url),
);
export const DEFAULT_DATABASE_PATH = join(GAME_ASSETS_PATH, 'db');

export interface CreateAppOptions {
  databasePath?: string;
  databaseRepository?: DatabaseRepositoryContract;
  mediaSourceRepository?: MediaSourceRepositoryContract;
  gameAssetsPath?: string;
}

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

  const status = isHttpError(error) ? error.status : getErrorStatus(error);
  if (status >= 500) {
    console.error('CEditor2 server error:', error);
  }

  response.status(status).json({
    error: isHttpError(error)
      ? error.message
      : status === 400
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

export function createApp(options: CreateAppOptions = {}): express.Express {
  const app = express();
  const databaseRepository =
    options.databaseRepository ??
    new DatabaseRepository(options.databasePath ?? DEFAULT_DATABASE_PATH);
  const mediaSourceRepository =
    options.mediaSourceRepository ??
    new MediaSourceRepository(options.gameAssetsPath ?? GAME_ASSETS_PATH);

  app.disable('x-powered-by');
  app.use(express.json({ limit: '50mb' }));

  app.get('/api/health', (_request, response) => {
    response.json({ status: 'ok' });
  });

  app.get('/api/database', async (_request, response) => {
    response.json(await databaseRepository.load());
  });

  app.put('/api/database', async (request, response) => {
    response.json(await databaseRepository.save(request.body));
  });

  app.get('/api/media-sources', async (_request, response) => {
    response.json(await mediaSourceRepository.load());
  });

  app.use(
    '/game-assets',
    express.static(options.gameAssetsPath ?? GAME_ASSETS_PATH, {
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
