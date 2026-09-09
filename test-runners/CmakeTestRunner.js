#!/usr/bin/env node

const { spawnSync } = require('child_process');
const path = require('path');

function invoke(command, args, cwd) {
  const result = spawnSync(command, args, { cwd, stdio: 'inherit', shell: false });
  if (result.error) {
    console.error(result.error.message);
  }
  if (result.status !== 0) {
    process.exit(result.status ?? 1);
  }
}

function defaultPreset() {
  return process.env.MSYSTEM === 'UCRT64' ? 'ucrt64-debug' : 'gcc-debug';
}

function runTest(testName, isUi, rawArgs) {
  if (!testName) {
    console.error('A test name is required');
    process.exit(1);
  }

  const root = path.resolve(__dirname, '..');
  const preset = process.env.CARCER_CMAKE_PRESET || defaultPreset();
  const target = `carcer_test_${testName}`;
  const executable = path.join(
    root,
    'build',
    'cmake',
    preset,
    target + (process.platform === 'win32' ? '.exe' : '')
  );
  const buildOnly = rawArgs.includes('--build-only');
  const testArgs = rawArgs.filter(arg => arg !== '--build-only');

  invoke('cmake', ['--preset', preset], root);
  invoke('cmake', ['--build', '--preset', preset, '--target', target], root);
  if (!buildOnly) {
    invoke(executable, isUi ? testArgs : [], path.join(root, 'src'));
  }
}

module.exports = { runTest };
