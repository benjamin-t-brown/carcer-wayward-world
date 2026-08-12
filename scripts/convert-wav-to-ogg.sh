#!/bin/bash

# Convert all .wav files under src/assets/snd to compressed Ogg Vorbis (.ogg).
# Requires ffmpeg with libvorbis.
#
# Usage (from repo root):
#   ./scripts/convert-wav-to-ogg.sh
#   ./scripts/convert-wav-to-ogg.sh --delete-wav --update-assets
#   .\scripts\Invoke-Ucrt64.ps1 "./scripts/convert-wav-to-ogg.sh --delete-wav --update-assets"
#
# Options:
#   --quality N       Vorbis quality 0-10 (default: 4; higher = larger/better)
#   --delete-wav      Remove each .wav after a successful conversion
#   --update-assets   Rewrite Sound paths in src/assets/assets.game.txt (.wav -> .ogg)
#                     Usually unnecessary: prefer AssetLoader::setSoundFileMode(SOUND_FILE_OGG)
#   --dry-run         Print actions without writing files

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
TARGET_DIR="$PROJECT_ROOT/src/assets/snd"
ASSETS_FILE="$PROJECT_ROOT/src/assets/assets.game.txt"

QUALITY=4
DELETE_WAV=0
UPDATE_ASSETS=0
DRY_RUN=0

usage() {
  sed -n '2,16p' "$0" | sed 's/^# \{0,1\}//'
  exit "${1:-0}"
}

while [ $# -gt 0 ]; do
  case "$1" in
    --quality)
      QUALITY="${2:-}"
      if [ -z "$QUALITY" ]; then
        echo "Error: --quality requires a value (0-10)."
        exit 1
      fi
      shift 2
      ;;
    --delete-wav)
      DELETE_WAV=1
      shift
      ;;
    --update-assets)
      UPDATE_ASSETS=1
      shift
      ;;
    --dry-run)
      DRY_RUN=1
      shift
      ;;
    -h|--help)
      usage 0
      ;;
    *)
      echo "Error: unknown option: $1"
      usage 1
      ;;
  esac
done

to_unix_path() {
  local path="$1"
  if command -v cygpath >/dev/null 2>&1; then
    cygpath -u "$path"
  else
    echo "$path"
  fi
}

resolve_ffmpeg() {
  if command -v ffmpeg >/dev/null 2>&1; then
    command -v ffmpeg
    return 0
  fi

  # MSYS2/Git Bash often do not inherit the Windows PATH; ask PowerShell.
  if command -v powershell.exe >/dev/null 2>&1; then
    local win_path
    win_path="$(powershell.exe -NoProfile -Command \
      "(Get-Command ffmpeg -ErrorAction SilentlyContinue).Source" 2>/dev/null | tr -d '\r')"
    if [ -n "$win_path" ]; then
      to_unix_path "$win_path"
      return 0
    fi
  fi

  local candidates=()
  if [ -n "${LOCALAPPDATA:-}" ]; then
    candidates+=("$(to_unix_path "$LOCALAPPDATA/Microsoft/WinGet/Links/ffmpeg.exe")")
  fi
  if [ -n "${USERPROFILE:-}" ]; then
    candidates+=("$(to_unix_path "$USERPROFILE/AppData/Local/Microsoft/WinGet/Links/ffmpeg.exe")")
  fi
  candidates+=(
    "/ucrt64/bin/ffmpeg"
    "/mingw64/bin/ffmpeg"
  )

  local candidate
  for candidate in "${candidates[@]}"; do
    if [ -n "$candidate" ] && [ -x "$candidate" ]; then
      echo "$candidate"
      return 0
    fi
  done

  return 1
}

FFMPEG="$(resolve_ffmpeg)" || {
  echo "Error: ffmpeg not found on PATH."
  echo "Install ffmpeg (e.g. winget install ffmpeg, or pacman -S mingw-w64-ucrt-x86_64-ffmpeg), then re-run."
  exit 1
}

if [ ! -d "$TARGET_DIR" ]; then
  echo "Error: Directory not found: $TARGET_DIR"
  exit 1
fi

echo "Converting WAV -> OGG in: $TARGET_DIR"
echo "Using: $FFMPEG"
echo "Quality: $QUALITY"
[ "$DELETE_WAV" -eq 1 ] && echo "Will delete source .wav after success"
[ "$UPDATE_ASSETS" -eq 1 ] && echo "Will update: $ASSETS_FILE"
[ "$DRY_RUN" -eq 1 ] && echo "Dry run (no writes)"
echo ""

CONVERTED=0
FAILED=0
DELETED=0

while IFS= read -r -d '' wav_file; do
  dir="$(dirname "$wav_file")"
  base="$(basename "$wav_file")"
  stem="${base%.*}"
  ogg_file="$dir/$stem.ogg"
  rel_wav="${wav_file#$TARGET_DIR/}"
  rel_ogg="${ogg_file#$TARGET_DIR/}"

  echo "Converting: $rel_wav -> $rel_ogg"

  if [ "$DRY_RUN" -eq 1 ]; then
    CONVERTED=$((CONVERTED + 1))
    if [ "$DELETE_WAV" -eq 1 ]; then
      echo "  (dry-run) would delete $rel_wav"
      DELETED=$((DELETED + 1))
    fi
    continue
  fi

  if "$FFMPEG" -y -hide_banner -loglevel error \
    -i "$wav_file" \
    -c:a libvorbis \
    -q:a "$QUALITY" \
    -map_metadata -1 \
    "$ogg_file"; then
    echo "  OK"
    CONVERTED=$((CONVERTED + 1))

    if [ "$DELETE_WAV" -eq 1 ]; then
      if rm -f "$wav_file"; then
        echo "  Deleted $rel_wav"
        DELETED=$((DELETED + 1))
      else
        echo "  Warning: failed to delete $rel_wav"
      fi
    fi
  else
    echo "  Failed"
    rm -f "$ogg_file"
    FAILED=$((FAILED + 1))
  fi
done < <(find "$TARGET_DIR" -type f -iname '*.wav' -print0 | sort -z)

if [ "$UPDATE_ASSETS" -eq 1 ]; then
  echo ""
  if [ ! -f "$ASSETS_FILE" ]; then
    echo "Warning: assets file not found: $ASSETS_FILE"
  elif [ "$DRY_RUN" -eq 1 ]; then
    count="$(grep -cE '^Sound,[^,]*,[^,]*\.wav(,|$)' "$ASSETS_FILE" || true)"
    echo "(dry-run) would update $count Sound path(s) in assets.game.txt"
  else
    tmp_file="${ASSETS_FILE}.tmp"
    # Only rewrite the path field of Sound lines (…assets/….wav -> …assets/….ogg).
    sed -E 's/^(Sound,[^,]*,[^,]*)\.wav(,|$)/\1.ogg\2/' "$ASSETS_FILE" > "$tmp_file"
    if mv -f "$tmp_file" "$ASSETS_FILE"; then
      echo "Updated Sound paths in assets.game.txt (.wav -> .ogg)"
    else
      echo "Error: failed to update $ASSETS_FILE"
      rm -f "$tmp_file"
      exit 1
    fi
  fi
fi

echo ""
echo "Done. Converted: $CONVERTED  Failed: $FAILED  Deleted wav: $DELETED"

if [ "$FAILED" -gt 0 ]; then
  exit 1
fi
