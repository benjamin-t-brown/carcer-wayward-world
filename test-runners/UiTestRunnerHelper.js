#!/usr/bin/env node

const { runTest } = require('./CmakeTestRunner');

const args = process.argv.slice(2);
if (args.length < 2) {
  console.error('Usage: node UiTestRunnerHelper.js <folder> <test-name> [--build-only] [args...]');
  process.exit(1);
}

runTest(args[1], true, args.slice(2));
