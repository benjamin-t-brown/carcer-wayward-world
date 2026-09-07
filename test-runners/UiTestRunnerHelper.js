#!/usr/bin/env node

/** Build and optionally run one UI CMake test target. */
const { spawnSync } = require('child_process');
const path = require('path');

const args = process.argv.slice(2);
if (args.length < 2) {
  console.error('Usage: node UiTestRunnerHelper.js <folder> <test-name> [--build-only]');
  process.exit(1);
}

const testName = args[1];
const buildOnly = args.includes('--build-only');
const testArgs = args.slice(2).filter(arg => arg !== '--build-only');
const root = path.resolve(__dirname, '..');
const sourceDir = path.join(root, 'src');
const preset = process.env.CARCER_CMAKE_PRESET ||
  (process.platform === 'win32' ? 'ucrt64-debug' : 'gcc-debug');
const target = `carcer_test_${testName}`;
const executable = path.join(
  root,
  'build',
  'cmake',
  preset,
  target + (process.platform === 'win32' ? '.exe' : '')
);

function run(command, commandArgs, cwd = root) {
  const result = spawnSync(command, commandArgs, { cwd, stdio: 'inherit', shell: false });
  if (result.error) console.error(result.error.message);
  if (result.status !== 0) process.exit(result.status ?? 1);
}

run('cmake', ['--preset', preset]);
run('cmake', ['--build', '--preset', preset, '--target', target]);
if (!buildOnly) run(executable, testArgs, sourceDir);
