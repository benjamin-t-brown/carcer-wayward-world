import {
  createContext,
  useCallback,
  useContext,
  useEffect,
  useMemo,
  useRef,
  useState,
  type ReactNode,
} from 'react';
import {
  EMPTY_ASSET_ORGANIZATION,
  copyPlacement,
  createFolder,
  deleteFolder as removeFolderFromTree,
  filingFolderId as filingFolderFor,
  findParentId,
  moveFolder,
  moveItem,
  organizationsEqual,
  pruneMissing,
  rekeyItem,
  removeItem,
  renameFolder,
  withSortedPlacements,
  type AssetOrganization,
  type EditorOrganizationFile,
  type OrganizableAssetId,
} from '../../shared/editorOrganization';

const SAVE_DELAY_MS = 300;

interface OrganizationState {
  file: EditorOrganizationFile;
  selectedFolderByAsset: Partial<Record<OrganizableAssetId, string | null>>;
}

interface OrganizationActions {
  updateSlice: (
    assetId: OrganizableAssetId,
    mutate: (org: AssetOrganization) => AssetOrganization,
  ) => void;
  selectFolder: (assetId: OrganizableAssetId, folderId: string | null) => void;
  selectedFolderId: (assetId: OrganizableAssetId) => string | null;
  organizationFor: (assetId: OrganizableAssetId) => AssetOrganization;
}

const OrganizationStateContext = createContext<OrganizationState | null>(null);
const OrganizationActionsContext = createContext<OrganizationActions | null>(null);

async function postOrganizationSlice(
  assetId: OrganizableAssetId,
  slice: AssetOrganization,
): Promise<void> {
  const response = await fetch(`/api/editor-organization/${assetId}`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(slice),
    keepalive: true,
  });
  if (!response.ok) {
    throw new Error(`Failed to save folder layout for ${assetId}`);
  }
}

export function OrganizationProvider({
  initialOrganization,
  children,
}: {
  initialOrganization: EditorOrganizationFile;
  children: ReactNode;
}) {
  const [file, setFile] = useState<EditorOrganizationFile>(initialOrganization);
  const [selectedFolderByAsset, setSelectedFolderByAsset] = useState<
    Partial<Record<OrganizableAssetId, string | null>>
  >({});
  const fileRef = useRef(file);
  fileRef.current = file;
  const selectedFolderRef = useRef(selectedFolderByAsset);
  selectedFolderRef.current = selectedFolderByAsset;
  const pendingSaves = useRef(new Map<OrganizableAssetId, AssetOrganization>());
  const saveTimers = useRef(new Map<OrganizableAssetId, number>());

  const flushPending = useCallback(() => {
    for (const timer of saveTimers.current.values()) {
      window.clearTimeout(timer);
    }
    saveTimers.current.clear();
    for (const [assetId, slice] of pendingSaves.current) {
      void postOrganizationSlice(assetId, slice).catch((error: unknown) => {
        console.error(error);
      });
    }
    pendingSaves.current.clear();
  }, []);

  const scheduleSave = useCallback((assetId: OrganizableAssetId, slice: AssetOrganization) => {
    pendingSaves.current.set(assetId, slice);
    const existing = saveTimers.current.get(assetId);
    if (existing != null) {
      window.clearTimeout(existing);
    }
    const timer = window.setTimeout(() => {
      saveTimers.current.delete(assetId);
      const body = pendingSaves.current.get(assetId);
      pendingSaves.current.delete(assetId);
      if (!body) {
        return;
      }
      void postOrganizationSlice(assetId, body).catch((error: unknown) => {
        console.error(error);
      });
    }, SAVE_DELAY_MS);
    saveTimers.current.set(assetId, timer);
  }, []);

  useEffect(() => {
    const onHide = () => {
      if (document.visibilityState === 'hidden') {
        flushPending();
      }
    };
    window.addEventListener('beforeunload', flushPending);
    document.addEventListener('visibilitychange', onHide);
    return () => {
      window.removeEventListener('beforeunload', flushPending);
      document.removeEventListener('visibilitychange', onHide);
    };
  }, [flushPending]);

  const actions = useMemo<OrganizationActions>(() => {
    return {
      organizationFor: (assetId) =>
        fileRef.current[assetId] ?? EMPTY_ASSET_ORGANIZATION,
      selectedFolderId: (assetId) => selectedFolderRef.current[assetId] ?? null,
      selectFolder: (assetId, folderId) => {
        selectedFolderRef.current = {
          ...selectedFolderRef.current,
          [assetId]: folderId,
        };
        setSelectedFolderByAsset(selectedFolderRef.current);
      },
      updateSlice: (assetId, mutate) => {
        const current = fileRef.current[assetId] ?? EMPTY_ASSET_ORGANIZATION;
        const nextSlice = withSortedPlacements(mutate(current));
        if (organizationsEqual(current, nextSlice)) {
          return;
        }
        const nextFile: EditorOrganizationFile = {
          ...fileRef.current,
          [assetId]: nextSlice,
        };
        fileRef.current = nextFile;
        setFile(nextFile);
        scheduleSave(assetId, nextSlice);
      },
    };
  }, [scheduleSave]);

  const state = useMemo<OrganizationState>(
    () => ({ file, selectedFolderByAsset }),
    [file, selectedFolderByAsset],
  );

  return (
    <OrganizationActionsContext.Provider value={actions}>
      <OrganizationStateContext.Provider value={state}>
        {children}
      </OrganizationStateContext.Provider>
    </OrganizationActionsContext.Provider>
  );
}

export interface AssetOrganizationApi {
  organization: AssetOrganization;
  selectedFolderId: string | null;
  selectFolder: (folderId: string | null) => void;
  createFolder: (parentId: string | null) => string;
  renameFolder: (folderId: string, name: string) => void;
  deleteFolder: (folderId: string) => void;
  moveItem: (itemId: string, folderId: string | null) => void;
  moveFolder: (folderId: string, newParentId: string | null) => void;
  rekeyItem: (oldId: string, newId: string) => void;
  removeItem: (itemId: string) => void;
  copyPlacement: (sourceId: string, newId: string) => void;
  fileNewItem: (itemId: string, selectedItemId: string | null) => void;
  pruneMissing: (liveIds: readonly string[]) => void;
  filingFolderId: (selectedItemId: string | null) => string | null;
}

export function useAssetOrganization(
  assetId: OrganizableAssetId,
): AssetOrganizationApi {
  const state = useContext(OrganizationStateContext);
  const actions = useContext(OrganizationActionsContext);
  if (!state || !actions) {
    throw new Error('useAssetOrganization requires OrganizationProvider');
  }

  const organization = state.file[assetId] ?? EMPTY_ASSET_ORGANIZATION;
  const selectedFolderId = state.selectedFolderByAsset[assetId] ?? null;

  const prune = useCallback(
    (liveIds: readonly string[]) => {
      actions.updateSlice(assetId, (org) => pruneMissing(org, liveIds));
    },
    [actions, assetId],
  );

  return {
    organization,
    selectedFolderId,
    selectFolder: (folderId) => actions.selectFolder(assetId, folderId),
    createFolder: (parentId) => {
      const folderId = crypto.randomUUID();
      actions.updateSlice(assetId, (org) =>
        createFolder(org, parentId, folderId),
      );
      actions.selectFolder(assetId, folderId);
      return folderId;
    },
    renameFolder: (folderId, name) => {
      actions.updateSlice(assetId, (org) => renameFolder(org, folderId, name));
    },
    deleteFolder: (folderId) => {
      const org = actions.organizationFor(assetId);
      const parent = findParentId(org.folders, folderId);
      actions.updateSlice(assetId, (current) =>
        removeFolderFromTree(current, folderId),
      );
      if (actions.selectedFolderId(assetId) === folderId) {
        actions.selectFolder(assetId, parent ?? null);
      }
    },
    moveItem: (itemId, folderId) => {
      actions.updateSlice(assetId, (org) => moveItem(org, itemId, folderId));
    },
    moveFolder: (folderId, newParentId) => {
      actions.updateSlice(assetId, (org) => moveFolder(org, folderId, newParentId));
    },
    rekeyItem: (oldId, newId) => {
      if (oldId === newId) {
        return;
      }
      actions.updateSlice(assetId, (org) => rekeyItem(org, oldId, newId));
    },
    removeItem: (itemId) => {
      actions.updateSlice(assetId, (org) => removeItem(org, itemId));
    },
    copyPlacement: (sourceId, newId) => {
      actions.updateSlice(assetId, (org) => copyPlacement(org, sourceId, newId));
    },
    fileNewItem: (itemId, selectedItemId) => {
      const org = actions.organizationFor(assetId);
      const folderId = filingFolderFor(
        org,
        actions.selectedFolderId(assetId),
        selectedItemId,
      );
      actions.updateSlice(assetId, (current) => moveItem(current, itemId, folderId));
    },
    pruneMissing: prune,
    filingFolderId: (selectedItemId) =>
      filingFolderFor(
        actions.organizationFor(assetId),
        actions.selectedFolderId(assetId),
        selectedItemId,
      ),
  };
}
