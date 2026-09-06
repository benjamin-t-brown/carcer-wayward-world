#!/usr/bin/env bash
# Materialize and validate Carcer's pinned source dependencies.
#
# Default behavior clones missing repositories and validates existing ones.
# Existing repositories are never fetched or moved unless --repair is passed.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LOCK_FILE="${CARCER_DEPS_LOCK:-${ROOT}/deps.lock}"
DEPS_ROOT="${CARCER_DEPS_ROOT:-${ROOT}/.deps}"
MODE="bootstrap"
ALLOW_DIRTY="${CARCER_ALLOW_DIRTY_DEPS:-0}"

usage() {
  cat <<'EOF'
Usage: scripts/bootstrap-deps.sh [--check | --repair] [--allow-dirty]

  no option       Clone missing dependencies and validate existing checkouts.
  --check         Validate only; never clone, fetch, or checkout.
  --repair        Clone missing dependencies and move clean existing checkouts
                  to the revisions pinned in deps.lock.
  --allow-dirty   Permit tracked local changes while still requiring pinned HEADs.

Offline mirrors may be selected without changing deps.lock:
  CARCER_SDL2W_REPOSITORY=/path/to/sdl2w
  CARCER_BMIN_REPOSITORY=/path/to/bmin
EOF
}

while (($#)); do
  case "$1" in
    --check)
      MODE="check"
      ;;
    --repair)
      MODE="repair"
      ;;
    --allow-dirty)
      ALLOW_DIRTY="1"
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "error: unknown argument: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
  shift
done

if [[ ! -f "${LOCK_FILE}" ]]; then
  echo "error: dependency lock not found: ${LOCK_FILE}" >&2
  exit 1
fi

if ! command -v git >/dev/null 2>&1; then
  echo "error: git is required to bootstrap dependencies" >&2
  exit 1
fi

lock_record() {
  local name="$1"
  awk -v wanted="${name}" '$1 == wanted { print $1, $2, $3, $4 }' "${LOCK_FILE}"
}

clone_url_for() {
  local name="$1"
  local locked_url="$2"
  case "${name}" in
    sdl2w)
      printf '%s\n' "${CARCER_SDL2W_REPOSITORY:-${locked_url}}"
      ;;
    bmin)
      printf '%s\n' "${CARCER_BMIN_REPOSITORY:-${locked_url}}"
      ;;
    *)
      printf '%s\n' "${locked_url}"
      ;;
  esac
}

is_tracked_tree_clean() {
  local checkout="$1"
  git -C "${checkout}" diff --quiet --ignore-submodules -- &&
    git -C "${checkout}" diff --cached --quiet --ignore-submodules --
}

validate_checkout() {
  local name="$1"
  local checkout="$2"
  local expected_commit="$3"
  local actual_commit

  if [[ ! -d "${checkout}/.git" ]]; then
    echo "error: ${name} checkout is missing or is not a Git repository: ${checkout}" >&2
    return 1
  fi

  actual_commit="$(git -C "${checkout}" rev-parse HEAD)"
  if [[ "${actual_commit}" != "${expected_commit}" ]]; then
    echo "error: ${name} is at ${actual_commit}, expected ${expected_commit}" >&2
    echo "Run scripts/bootstrap-deps.sh --repair to select the pinned revision." >&2
    return 1
  fi

  if [[ "${ALLOW_DIRTY}" != "1" ]] && ! is_tracked_tree_clean "${checkout}"; then
    echo "error: ${name} has tracked local changes: ${checkout}" >&2
    echo "Commit/stash them, or explicitly pass --allow-dirty for dependency development." >&2
    return 1
  fi

  if [[ "${ALLOW_DIRTY}" == "1" ]] && ! is_tracked_tree_clean "${checkout}"; then
    echo "warning: allowing tracked local changes in ${name}" >&2
  fi

  echo "Validated ${name} at ${expected_commit}"
}

prepare_checkout() {
  local name="$1"
  local locked_url="$2"
  local locked_ref="$3"
  local expected_commit="$4"
  local checkout="${DEPS_ROOT}/${name}"
  local clone_url
  clone_url="$(clone_url_for "${name}" "${locked_url}")"

  if [[ ! -e "${checkout}" ]]; then
    if [[ "${MODE}" == "check" ]]; then
      echo "error: ${name} is not bootstrapped at ${checkout}" >&2
      echo "Run scripts/bootstrap-deps.sh first." >&2
      return 1
    fi

    mkdir -p "${DEPS_ROOT}"
    echo "Cloning ${name} from ${clone_url}"
    git clone --no-checkout "${clone_url}" "${checkout}"
    if ! git -C "${checkout}" cat-file -e "${expected_commit}^{commit}" 2>/dev/null; then
      git -C "${checkout}" fetch --no-tags origin "${locked_ref}"
    fi
    git -C "${checkout}" checkout --detach "${expected_commit}"
  elif [[ ! -d "${checkout}/.git" ]]; then
    echo "error: refusing to replace non-Git path: ${checkout}" >&2
    return 1
  elif [[ "${MODE}" == "repair" ]]; then
    local actual_commit
    actual_commit="$(git -C "${checkout}" rev-parse HEAD)"
    if [[ "${actual_commit}" != "${expected_commit}" ]]; then
      if ! is_tracked_tree_clean "${checkout}"; then
        echo "error: refusing to move dirty ${name} checkout: ${checkout}" >&2
        return 1
      fi
      echo "Fetching pinned ${name} revision from ${clone_url}"
      git -C "${checkout}" remote set-url origin "${clone_url}"
      git -C "${checkout}" fetch --no-tags origin "${locked_ref}"
      git -C "${checkout}" checkout --detach "${expected_commit}"
    fi
  fi

  validate_checkout "${name}" "${checkout}" "${expected_commit}"
}

for dependency in sdl2w bmin; do
  record="$(lock_record "${dependency}")"
  if [[ -z "${record}" ]]; then
    echo "error: deps.lock has no ${dependency} record" >&2
    exit 1
  fi
  read -r name repository ref commit <<<"${record}"
  if [[ ! "${commit}" =~ ^[0-9a-fA-F]{40}$ ]]; then
    echo "error: invalid pinned commit for ${name}: ${commit}" >&2
    exit 1
  fi
  prepare_checkout "${name}" "${repository}" "${ref}" "${commit}"
done
