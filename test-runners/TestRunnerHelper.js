#!/usr/bin/env node

const { runTest } = require('./CmakeTestRunner');

const args = process.argv.slice(2);
if (args.length < 3) {
  console.error('Usage: node TestRunnerHelper.js <parent> <folder> <test-name> [--build-only]');
  process.exit(1);
}

runTest(args[2], false, args.slice(3));
