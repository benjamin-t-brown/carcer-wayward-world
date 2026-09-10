import cors from 'cors';
import express, {
  type ErrorRequestHandler,
  type Express,
  type Request,
  type RequestHandler,
  type Response,
} from 'express';
import fs, { readdir } from 'fs/promises';
import { dirname, isAbsolute, join, relative, resolve, sep } from 'path';
import { fileURLToPath } from 'url';
import { ASSET_TYPES, assetFileForId } from '../shared/assetRegistry';
import {
  DatabaseRepository,
  type DatabaseRepositoryContract,
} from './databaseRepository';
import { BadRequestError, isHttpError } from './httpErrors';

export interface ServerPaths {
  projectRoot: string;
  assetsDatabasePath: string;
  assetsPath: string;
  sourcePath: string;
  terrainEdgesPath: string;
}

export interface CreateAppOptions {
  /** Convenience override used by repository/API integration tests. */
  databasePath?: string;
  repository?: DatabaseRepositoryContract;
  paths?: Partial<ServerPaths>;
  logger?: Pick<Console, 'error' | 'log'>;
}

type AsyncRequestHandler = (req: Request, res: Response) => Promise<unknown>;

/** Resolve production paths relative to this server module, independent of cwd. */
export function defaultServerPaths(moduleUrl = import.meta.url): ServerPaths {
  const serverDirectory = dirname(fileURLToPath(moduleUrl));
  const projectRoot = resolve(serverDirectory, '../../..');
  const assetsPath = join(projectRoot, 'src/assets');
  return {
    projectRoot,
    assetsDatabasePath: join(assetsPath, 'db'),
    assetsPath,
    sourcePath: join(projectRoot, 'src'),
    terrainEdgesPath: join(serverDirectory, '../client/terrain-edges.html'),
  };
}

function resolveServerPaths(overrides: Partial<ServerPaths> = {}): ServerPaths {
  const defaults = defaultServerPaths();
  const projectRoot = overrides.projectRoot ?? defaults.projectRoot;
  return {
    projectRoot,
    assetsDatabasePath:
      overrides.assetsDatabasePath ?? join(projectRoot, 'src/assets/db'),
    assetsPath: overrides.assetsPath ?? join(projectRoot, 'src/assets'),
    sourcePath: overrides.sourcePath ?? join(projectRoot, 'src'),
    terrainEdgesPath:
      overrides.terrainEdgesPath ??
      (overrides.projectRoot
        ? join(projectRoot, 'ceditor/src/client/terrain-edges.html')
        : defaults.terrainEdgesPath),
  };
}

function asyncRoute(handler: AsyncRequestHandler): RequestHandler {
  return (req, res, next) => {
    void handler(req, res).catch(next);
  };
}

async function getSdl2wAssetFileContents(
  assetsPath: string,
): Promise<Record<string, string>> {
  const files = await readdir(assetsPath);
  const assetFiles = files.filter(
    (file) => file.startsWith('assets.') && file.endsWith('.txt'),
  );
  const contents: Record<string, string> = {};
  for (const file of assetFiles) {
    contents[file] = await fs.readFile(join(assetsPath, file), 'utf-8');
  }
  return contents;
}

function contentTypeForAssetFile(filePath: string): string {
  const extension = filePath.split('.').pop()?.toLowerCase();
  switch (extension) {
    case 'png':
      return 'image/png';
    case 'jpg':
    case 'jpeg':
      return 'image/jpeg';
    case 'gif':
      return 'image/gif';
    case 'webp':
      return 'image/webp';
    case 'wav':
      return 'audio/wav';
    case 'ogg':
      return 'audio/ogg';
    case 'mp3':
      return 'audio/mpeg';
    default:
      return 'application/octet-stream';
  }
}

function pathInside(root: string, requestedPath: string): string | null {
  const filePath = resolve(root, requestedPath);
  const relativePath = relative(root, filePath);
  if (
    relativePath === '..' ||
    relativePath.startsWith(`..${sep}`) ||
    isAbsolute(relativePath)
  ) {
    return null;
  }
  return filePath;
}

async function serveSourceAsset(
  req: Request,
  res: Response,
  sourcePath: string,
  notFoundLabel: string,
): Promise<void> {
  // `/api/` is a transport prefix; the remainder is relative to game `src/`.
  const relativePath = req.path.replace(/^\/api\//, '');
  const filePath = pathInside(sourcePath, relativePath);
  if (!filePath) {
    res.status(403).json({ error: 'Invalid path' });
    return;
  }

  try {
    const fileBuffer = await fs.readFile(filePath);
    res.setHeader('Content-Type', contentTypeForAssetFile(filePath));
    res.send(fileBuffer);
  } catch (error) {
    if (
      typeof error === 'object' &&
      error !== null &&
      'code' in error &&
      error.code === 'ENOENT'
    ) {
      res.status(404).json({ error: `${notFoundLabel} not found` });
      return;
    }
    throw error;
  }
}

function requestErrorStatus(error: unknown): number {
  if (isHttpError(error)) {
    return error.status;
  }
  if (
    typeof error === 'object' &&
    error !== null &&
    'status' in error &&
    error.status === 400
  ) {
    return 400;
  }
  return 500;
}

/** Construct the CEditor HTTP application without opening a listening socket. */
export function createApp(options: CreateAppOptions = {}): Express {
  const paths = resolveServerPaths({
    ...options.paths,
    ...(options.databasePath
      ? { assetsDatabasePath: options.databasePath }
      : {}),
  });
  const repository =
    options.repository ?? new DatabaseRepository(paths.assetsDatabasePath);
  const logger = options.logger ?? console;
  const app = express();

  app.use(cors());
  app.use(express.json({ limit: '50mb' }));
  app.use('/assets', express.static(paths.sourcePath));
  app.get('/terrain-edges.html', (_req, res) => {
    res.sendFile(paths.terrainEdgesPath);
  });

  app.get(
    '/api/database',
    asyncRoute(async (_req, res) => {
      res.json(await repository.load());
    }),
  );

  app.put(
    '/api/database',
    asyncRoute(async (req, res) => {
      res.json(await repository.save(req.body));
    }),
  );

  app.get('/api/assets/types', (_req, res) => {
    res.json(ASSET_TYPES);
  });

  // Legacy collection endpoints remain available while clients migrate to the
  // coherent database envelope.
  app.get(
    '/api/assets/:type',
    asyncRoute(async (req, res) => {
      const fileName = assetFileForId(req.params.type);
      if (!fileName) {
        throw new BadRequestError('Invalid asset type');
      }
      const content = await fs.readFile(
        join(paths.assetsDatabasePath, fileName),
        'utf-8',
      );
      res.json(JSON.parse(content));
    }),
  );

  app.post(
    '/api/assets/:type',
    asyncRoute(async (req, res) => {
      const fileName = assetFileForId(req.params.type);
      if (!fileName) {
        throw new BadRequestError('Invalid asset type');
      }
      await fs.mkdir(paths.assetsDatabasePath, { recursive: true });
      const content = JSON.stringify(req.body, null, 2);
      logger.log(
        `saving asset ${req.params.type} (${Buffer.byteLength(content)} bytes)`,
      );
      await fs.writeFile(
        join(paths.assetsDatabasePath, fileName),
        content,
        'utf-8',
      );
      res.json({ success: true });
    }),
  );

  app.get(
    '/api/sdl2w-assets',
    asyncRoute(async (_req, res) => {
      res.json(await getSdl2wAssetFileContents(paths.assetsPath));
    }),
  );

  app.get(
    '/api/assets/img/*',
    asyncRoute(async (req, res) => {
      await serveSourceAsset(req, res, paths.sourcePath, 'Image');
    }),
  );

  app.get(
    '/api/assets/snd/*',
    asyncRoute(async (req, res) => {
      await serveSourceAsset(req, res, paths.sourcePath, 'Sound');
    }),
  );

  const errorHandler: ErrorRequestHandler = (error, _req, res, _next) => {
    const status = requestErrorStatus(error);
    if (status === 500) {
      logger.error('CEditor server request failed:', error);
    }
    const message =
      status === 500
        ? 'Internal server error'
        : error instanceof Error
          ? error.message
          : 'Bad request';
    res.status(status).json({ error: message });
  };
  app.use(errorHandler);

  return app;
}
