import { formatMapBenchmark, runMapBenchmark } from './map-render-harness.js';

function integerArgument(name: string, fallback: number): number {
  const prefix = `--${name}=`;
  const argument = globalThis.process.argv.find((value) =>
    value.startsWith(prefix),
  );
  if (!argument) {
    return fallback;
  }
  const value = Number(argument.slice(prefix.length));
  if (!Number.isSafeInteger(value) || value < 0) {
    throw new RangeError(`${name} must be a non-negative integer`);
  }
  return value;
}

const report = await runMapBenchmark({
  warmupFrames: integerArgument('warmup', 100),
  measuredFrames: integerArgument('frames', 500),
});
const json = globalThis.process.argv.includes('--json');
globalThis.process.stdout.write(
  `${json ? JSON.stringify(report, null, 2) : formatMapBenchmark(report)}\n`,
);
