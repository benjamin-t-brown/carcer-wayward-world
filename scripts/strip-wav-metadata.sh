#!/bin/bash

# Strip metadata (RIFF INFO/LIST, tags, etc.) from all .wav files under src/assets/snd.
# Uses ffmpeg with stream copy so sample format/rate/channels are unchanged.
#
# Usage (from repo root):
#   ./scripts/strip-wav-metadata.sh
#   .\scripts\Invoke-Ucrt64.ps1 "./scripts/strip-wav-metadata.sh"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
TARGET_DIR="$PROJECT_ROOT/src/assets/snd"

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

  # Common WinGet shim location
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

echo "Stripping WAV metadata in: $TARGET_DIR"
echo "Using: $FFMPEG"
echo ""

STRIPPED=0
FAILED=0

while IFS= read -r -d '' wav_file; do
  tmp_file="${wav_file}.tmp.wav"
  rel_path="${wav_file#$TARGET_DIR/}"

  echo "Processing: $rel_path"

  if "$FFMPEG" -y -hide_banner -loglevel error \
    -i "$wav_file" \
    -map_metadata -1 \
    -fflags +bitexact \
    -flags:a +bitexact \
    -c:a copy \
    "$tmp_file"; then
    if mv -f "$tmp_file" "$wav_file"; then
      echo "  OK"
      STRIPPED=$((STRIPPED + 1))
    else
      echo "  Failed to replace original"
      rm -f "$tmp_file"
      FAILED=$((FAILED + 1))
    fi
  else
    echo "  Failed"
    rm -f "$tmp_file"
    FAILED=$((FAILED + 1))
  fi
done < <(find "$TARGET_DIR" -type f -iname '*.wav' -print0 | sort -z)

echo ""
echo "Done. Stripped: $STRIPPED  Failed: $FAILED"

if [ "$FAILED" -gt 0 ]; then
  exit 1
fi
