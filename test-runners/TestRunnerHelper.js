#!/usr/bin/env node

/**
 * Generic UI Test Runner Helper
 * Usage: node UiTestRunnerHelper.js <folder> <test_file_name> [--build-only]
 *
 * Arguments:
 *   folder: Folder containing the test (Elements or Components)
 *   test_file_name: Name of the test file (without .cpp extension)
 *   --build-only: Optional flag to only build the test without running it
 */

const { execSync } = require('child_process');
const path = require('path');
const fs = require('fs');

// Get command-line arguments
const args = process.argv.slice(2);

if (args.length < 2) {
  console.error('Error:parent folder, folder and test file name are required');
  console.error('Usage: node TestRunnerHelper.js <parent folder><folder> <test_file_name> [--build-only]');
  process.exit(1);
}

const PARENT_FOLDER = args[0];
const TEST_FOLDER = args[1];
const TEST_FILE_NAME = args[2];
const TEST_EXE_NAME = 'TestUi';
const BUILD_ONLY = args.includes('--build-only');

const SCRIPT_DIR = __dirname;
const TOP_LEVEL_DIR = path.resolve(SCRIPT_DIR, '..');
const SRC_DIR = path.join(TOP_LEVEL_DIR, 'src');

// Clean up old test executables
const testExePattern = new RegExp(`^${TEST_EXE_NAME}\\.`);
try {
  const files = fs.readdirSync(SRC_DIR);
  files.forEach(file => {
    if (file.startsWith(TEST_EXE_NAME + '.') || file === TEST_EXE_NAME) {
      const filePath = path.join(SRC_DIR, file);
      fs.unlinkSync(filePath);
    }
  });
} catch (err) {
  // Ignore errors if files don't exist
}

// Compile objects
console.log('compile objects');
try {
  execSync('make object_files -j8', { cwd: SRC_DIR, stdio: 'inherit' });
} catch (err) {
  console.error('Failed to compile objects');
  process.exit(1);
}

// Compile test
console.log('compile test');
let compilerArgs;
try {
  compilerArgs = execSync('make compiler_args', { cwd: SRC_DIR, encoding: 'utf8' }).trim();
} catch (err) {
  console.error('Failed to get compiler args');
  process.exit(1);
}

const COMPILE_TEST = `g++ __test__/${PARENT_FOLDER}/${TEST_FOLDER}/${TEST_FILE_NAME}.cpp ${compilerArgs} -o ${TEST_EXE_NAME}`;
console.log(COMPILE_TEST);

try {
  execSync(COMPILE_TEST, { cwd: SRC_DIR, stdio: 'inherit' });
} catch (err) {
  console.error('Failed to compile test');
  process.exit(1);
}

// Run test (unless --build-only flag is set)
if (!BUILD_ONLY) {
  console.log('run test');
  try {
    execSync(`${SRC_DIR}/${TEST_EXE_NAME}`, { cwd: SRC_DIR, stdio: 'inherit' });
  } catch (err) {
    console.error('Test failed', err);
    process.exit(1);
  }
}

