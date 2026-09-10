import {
  copyFile,
  mkdir,
  mkdtemp,
  readFile,
  rename,
  rm,
  writeFile,
} from 'node:fs/promises';
import { basename, join } from 'node:path';

export interface SaveOutput {
  fileName: string;
  content: string;
}

export interface SaveTransactionOptions {
  databasePath: string;
  outputs: readonly SaveOutput[];
  currentContents: ReadonlyMap<string, string>;
  /** Test seam for simulating a failed replacement. */
  replaceFile?: (source: string, destination: string) => Promise<void>;
}

/**
 * Stage and parse-check a complete database snapshot, then replace only files
 * whose serialized bytes changed. Every changed target is backed up before the
 * first replacement so a partial commit can be rolled back.
 */
export async function commitSaveTransaction({
  databasePath,
  outputs,
  currentContents,
  replaceFile = rename,
}: SaveTransactionOptions): Promise<string[]> {
  assertSafeOutputs(outputs);
  await mkdir(databasePath, { recursive: true });

  const transactionPath = await mkdtemp(join(databasePath, '.ceditor2-save-'));
  const stagedPath = join(transactionPath, 'staged');
  const backupPath = join(transactionPath, 'backup');
  const changed = outputs.filter(
    ({ fileName, content }) => currentContents.get(fileName) !== content,
  );

  try {
    await mkdir(stagedPath);
    await mkdir(backupPath);

    for (const output of outputs) {
      const path = join(stagedPath, output.fileName);
      await writeFile(path, output.content, 'utf8');
      await assertStagedArray(path, output.fileName);
    }

    for (const { fileName } of changed) {
      await copyFile(join(databasePath, fileName), join(backupPath, fileName));
    }

    const replaced: string[] = [];
    try {
      for (const { fileName } of changed) {
        await replaceFile(
          join(stagedPath, fileName),
          join(databasePath, fileName),
        );
        replaced.push(fileName);
      }
    } catch (error) {
      await rollbackReplacements(databasePath, backupPath, replaced, error);
      throw error;
    }

    return changed.map(({ fileName }) => fileName);
  } finally {
    try {
      await rm(transactionPath, { recursive: true, force: true });
    } catch (error) {
      // A cleanup problem must not turn an already committed database save
      // into a reported failure. The transaction directory is hidden and can
      // be removed safely on a later run.
      console.error(
        `Could not remove database transaction directory ${transactionPath}:`,
        error,
      );
    }
  }
}

function assertSafeOutputs(outputs: readonly SaveOutput[]): void {
  const names = new Set<string>();
  for (const { fileName } of outputs) {
    if (!fileName || basename(fileName) !== fileName) {
      throw new Error(`Unsafe managed database filename: ${fileName}`);
    }
    if (names.has(fileName)) {
      throw new Error(`Duplicate managed database filename: ${fileName}`);
    }
    names.add(fileName);
  }
}

async function assertStagedArray(
  stagedFilePath: string,
  fileName: string,
): Promise<void> {
  const content = await readFile(stagedFilePath, 'utf8');
  const parsed: unknown = JSON.parse(content);
  if (!Array.isArray(parsed)) {
    throw new Error(`Staged database file is not an array: ${fileName}`);
  }
}

async function rollbackReplacements(
  databasePath: string,
  backupPath: string,
  replaced: readonly string[],
  commitError: unknown,
): Promise<void> {
  const rollbackErrors: unknown[] = [];
  for (const fileName of [...replaced].reverse()) {
    try {
      await copyFile(join(backupPath, fileName), join(databasePath, fileName));
    } catch (error) {
      rollbackErrors.push(error);
    }
  }

  if (rollbackErrors.length > 0) {
    throw new AggregateError(
      [commitError, ...rollbackErrors],
      'Database commit failed and rollback was incomplete',
    );
  }
}
