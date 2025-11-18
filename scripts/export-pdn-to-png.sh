#!/bin/bash

# Script to recursively find all .pdn files and export them as .png files
# Uses pdn2png.exe for conversion

SCRIPT_DIR="$(dirname "$0")"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
TARGET_DIR="$PROJECT_ROOT/src/assets/img"
CONVERTER="./pdn2png.exe"

# Check if converter exists
if [ ! -f "$CONVERTER" ]; then
    echo "Error: pdn2png.exe not found at: $CONVERTER"
    echo "Please ensure pdn2png.exe is in the scripts directory."
    exit 1
fi

# Check if target directory exists
if [ ! -d "$TARGET_DIR" ]; then
    echo "Error: Directory not found: $TARGET_DIR"
    echo "Please check the path and try again."
    exit 1
fi

echo "Using converter: $CONVERTER"
echo "Searching for .pdn files in: $TARGET_DIR"
echo ""

# Counter for converted files
CONVERTED=0
FAILED=0

# Find all .pdn files recursively and convert them
while IFS= read -r pdn_file; do
    # Get the directory and base name
    dir=$(dirname "$pdn_file")
    base=$(basename "$pdn_file" .pdn)
    png_file="$dir/$base.png"
    
    echo "Converting: $pdn_file"
    echo "  -> $png_file"
    
    # Convert using pdn2png.exe
    # Convert paths to Windows format if running in Git Bash
    if command -v cygpath >/dev/null 2>&1; then
        pdn_file_win="$(cygpath -w "$pdn_file")"
        png_file_win="$(cygpath -w "$png_file")"
        echo "Running: \"$CONVERTER\" \"$pdn_file_win\" \"$png_file_win\""
        if "$CONVERTER" "$pdn_file_win" "$png_file_win"; then
            echo "  ✓ Success"
            CONVERTED=$((CONVERTED + 1))
        else
            echo "  ✗ Failed"
            FAILED=$((FAILED + 1))
        fi
    else
        # For WSL or native Windows environments
        echo "Running: \"$CONVERTER\" \"$pdn_file\" \"$png_file\""
        if "$CONVERTER" "$pdn_file" "$png_file"; then
            echo "  ✓ Success"
            CONVERTED=$((CONVERTED + 1))
        else
            echo "  ✗ Failed"
            FAILED=$((FAILED + 1))
        fi
    fi
    echo ""
done < <(find "$TARGET_DIR" -type f -name "*.pdn")

echo "Conversion complete!"
echo "Converted: $CONVERTED files"
if [ $FAILED -gt 0 ]; then
    echo "Failed: $FAILED files"
fi
