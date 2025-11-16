#!/bin/bash

# Script to recursively find all .pdn files and export them as .png files
# Uses Paint.NET's command-line tool for conversion

SCRIPT_DIR="$(dirname "$0")"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
TARGET_DIR="$PROJECT_ROOT/game/src/assetr/img"

# Check if target directory exists
if [ ! -d "$TARGET_DIR" ]; then
    echo "Error: Directory not found: $TARGET_DIR"
    echo "Please check the path and try again."
    exit 1
fi

# Find Paint.NET executable in common locations
PAINTDOTNET=""
if [ -f "/c/Program Files/paint.net/PaintDotNet.exe" ]; then
    PAINTDOTNET="/c/Program Files/paint.net/PaintDotNet.exe"
elif [ -f "/c/Program Files (x86)/paint.net/PaintDotNet.exe" ]; then
    PAINTDOTNET="/c/Program Files (x86)/paint.net/PaintDotNet.exe"
elif [ -f "C:/Program Files/paint.net/PaintDotNet.exe" ]; then
    PAINTDOTNET="C:/Program Files/paint.net/PaintDotNet.exe"
elif [ -f "C:/Program Files (x86)/paint.net/PaintDotNet.exe" ]; then
    PAINTDOTNET="C:/Program Files (x86)/paint.net/PaintDotNet.exe"
else
    echo "Error: Paint.NET not found in common installation locations."
    echo "Please install Paint.NET or modify this script to point to your Paint.NET installation."
    exit 1
fi

echo "Using Paint.NET at: $PAINTDOTNET"
echo "Searching for .pdn files in: $TARGET_DIR"
echo ""

# Counter for converted files
CONVERTED=0
FAILED=0

# Find all .pdn files recursively and convert them
# Use process substitution to avoid subshell issues with counters
while IFS= read -r pdn_file; do
    # Get the directory and base name
    dir=$(dirname "$pdn_file")
    base=$(basename "$pdn_file" .pdn)
    png_file="$dir/$base.png"
    
    echo "Converting: $pdn_file"
    echo "  -> $png_file"
    
    # Convert using Paint.NET command-line tool
    # Paint.NET command-line format: PaintDotNet.exe /cmdline <input> <output>
    if "$PAINTDOTNET" /cmdline "$pdn_file" "$png_file"; then
        echo "  ✓ Success"
        CONVERTED=$((CONVERTED + 1))
    else
        echo "  ✗ Failed"
        FAILED=$((FAILED + 1))
    fi
    echo ""
done < <(find "$TARGET_DIR" -type f -name "*.pdn")

echo "Conversion complete!"
echo "Converted: $CONVERTED files"
if [ $FAILED -gt 0 ]; then
    echo "Failed: $FAILED files"
fi

