#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

min_version() {
  local current="$1"
  local minimum="$2"
  local IFS=.
  read -r c_major c_minor c_patch _ <<<"${current#v}"
  read -r m_major m_minor m_patch _ <<<"${minimum#v}"
  c_patch="${c_patch%%[^0-9]*}"
  m_patch="${m_patch%%[^0-9]*}"
  if (( c_major > m_major )); then return 0; fi
  if (( c_major < m_major )); then return 1; fi
  if (( c_minor > m_minor )); then return 0; fi
  if (( c_minor < m_minor )); then return 1; fi
  (( c_patch >= m_patch ))
}

if ! command -v npm >/dev/null 2>&1; then
  echo "npm is required but was not found on PATH." >&2
  exit 1
fi

npm_version="$(npm -v | tr -d '[:space:]')"
if ! min_version "$npm_version" "11.10.0"; then
  echo "npm 11.10.0 or newer is required (found $npm_version). .npmrc min-release-age needs npm >= 11.10.0." >&2
  exit 1
fi

echo "Installing ceditor dependencies with npm ci (lockfile only, scripts disabled via .npmrc)..."
npm ci
echo "ceditor dependencies installed."
