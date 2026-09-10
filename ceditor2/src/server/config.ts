export const SERVER_HOST = '127.0.0.1';
export const DEFAULT_SERVER_PORT = 3001;

export function resolveServerPort(
  configuredPort = process.env.CEDITOR2_SERVER_PORT,
): number {
  if (configuredPort === undefined || configuredPort.trim() === '') {
    return DEFAULT_SERVER_PORT;
  }

  const port = Number(configuredPort);
  if (!Number.isInteger(port) || port < 1 || port > 65_535) {
    throw new Error(
      `CEDITOR2_SERVER_PORT must be an integer between 1 and 65535; received ${JSON.stringify(configuredPort)}`,
    );
  }

  return port;
}
