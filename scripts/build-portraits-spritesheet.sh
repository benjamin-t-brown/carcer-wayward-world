#!/bin/bash

# Build portraits0.png from individual port_*.png files using ImageMagick.
#
# Portrait order is stored in portraits-order.txt beside the source images.
# Sources are port_*.png and port_*.pdn (stems deduplicated). Existing entries
# keep their slot index; newly discovered portraits are appended in alphabetical
# order so references like portraits0_5 stay stable.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SRC_DIR="$PROJECT_ROOT/src"
PORT_DIR="$PROJECT_ROOT/src/assets/img/game/port"
ORDER_FILE="$PORT_DIR/portraits-order.txt"
OUTPUT_FILE="$PROJECT_ROOT/src/assets/img/game/portraits0.png"
PIC_ALIAS="portraits0"

SPRITE_SIZE=52
COLS=8
ROWS=8
MAX_SPRITES=$((COLS * ROWS))
PORTRAIT_PREFIX="port_"

INIT_ORDER=0
LIST_ONLY=0

usage() {
    cat <<EOF
Usage: $(basename "$0") [--init-order] [--list]

  --init-order  Rebuild portraits-order.txt from current PNG/PDN files (alphabetical).
  --list        Print portrait index mapping without building the spritesheet.
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --init-order) INIT_ORDER=1 ;;
        --list) LIST_ONLY=1 ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
    shift
done

if [[ $LIST_ONLY -eq 0 ]] && ! command -v magick >/dev/null 2>&1; then
    echo "Error: ImageMagick 'magick' not found in PATH." >&2
    echo "Install ImageMagick and ensure magick.exe is on your PATH." >&2
    exit 1
fi

if [[ ! -d "$PORT_DIR" ]]; then
    echo "Error: portrait directory not found: $PORT_DIR" >&2
    exit 1
fi

discover_portraits() {
    PORTRAITS_DISCOVERED=()
    shopt -s nullglob
    local files=("$PORT_DIR"/${PORTRAIT_PREFIX}*.png "$PORT_DIR"/${PORTRAIT_PREFIX}*.pdn)
    shopt -u nullglob
    local stems=()
    for file in "${files[@]}"; do
        stems+=("$(basename "$file" | sed 's/\.[^.]*$//')")
    done
    if [[ ${#stems[@]} -gt 0 ]]; then
        IFS=$'\n' PORTRAITS_DISCOVERED=($(printf '%s\n' "${stems[@]}" | sort -u))
        unset IFS
    fi
}

read_order_file() {
    ORDER=()
    if [[ ! -f "$ORDER_FILE" ]]; then
        return
    fi
    while IFS= read -r line || [[ -n "$line" ]]; do
        line="${line%%#*}"
        line="${line#"${line%%[![:space:]]*}"}"
        line="${line%"${line##*[![:space:]]}"}"
        [[ -z "$line" ]] && continue
        ORDER+=("$line")
    done < "$ORDER_FILE"
}

write_order_file() {
  {
    echo "# Portrait spritesheet slot order (one stem per line, e.g. port_claire)."
    echo "# Existing entries keep their portraits0_N index; new portraits are appended."
    echo "# Reorder lines manually only when you intend to change sprite indices."
    echo ""
    for stem in "${ORDER[@]}"; do
        echo "$stem"
    done
    echo ""
  } > "$ORDER_FILE"
}

stem_is_discovered() {
    local stem="$1"
    local item
    for item in "${PORTRAITS_DISCOVERED[@]}"; do
        if [[ "$item" == "$stem" ]]; then
            return 0
        fi
    done
    return 1
}

sync_order() {
    local known=()
    local new_stems=()
    local stem item index

    for stem in "${ORDER[@]}"; do
        known+=("$stem")
        if ! stem_is_discovered "$stem"; then
            echo "Warning: portraits listed in order file but missing PNG/PDN:"
            echo "  - ${stem}.png"
        fi
    done

    for item in "${PORTRAITS_DISCOVERED[@]}"; do
        local found=0
        for stem in "${ORDER[@]}"; do
            if [[ "$stem" == "$item" ]]; then
                found=1
                break
            fi
        done
        if [[ $found -eq 0 ]]; then
            new_stems+=("$item")
        fi
    done

    if [[ ${#new_stems[@]} -eq 0 ]]; then
        return 1
    fi

    echo "Appended new portraits to order file:"
    for item in "${new_stems[@]}"; do
        ORDER+=("$item")
        index=$((${#ORDER[@]} - 1))
        echo "  - $item -> portraits0_$index"
    done
    write_order_file
    echo "Updated $ORDER_FILE"
    return 0
}

print_index_map() {
    local index stem name
    echo ""
    echo "Portrait indices:"
    for index in "${!ORDER[@]}"; do
        if [[ $index -ge $MAX_SPRITES ]]; then
            break
        fi
        stem="${ORDER[$index]}"
        name="${stem#${PORTRAIT_PREFIX}}"
        echo "  portraits0_${index}  ${name}"
    done
}

print_sdl2w_asset_lines() {
    local pic_path="${OUTPUT_FILE#$SRC_DIR/}"
    echo ""
    echo "SDL2W asset file entries (add to assets.game.txt):"
    echo "Pic,${PIC_ALIAS},${pic_path}"
    echo "Sprites,${PIC_ALIAS},${MAX_SPRITES},${SPRITE_SIZE},${SPRITE_SIZE}"
}

to_windows_path() {
    local path="$1"
    if command -v cygpath >/dev/null 2>&1; then
        cygpath -w "$path"
    else
        printf '%s' "$path"
    fi
}

validate_portrait_size() {
    local png="$1"
    local size
    size="$(magick identify -format "%wx%h" "$png")"
    if [[ "$size" != "${SPRITE_SIZE}x${SPRITE_SIZE}" ]]; then
        echo "Error: $(basename "$png") is ${size}, expected ${SPRITE_SIZE}x${SPRITE_SIZE}" >&2
        exit 1
    fi
}

build_spritesheet() {
    local temp_dir
    temp_dir="$(mktemp -d)"

    local empty_png="$temp_dir/empty.png"
    local montage_list="$temp_dir/montage.txt"
    magick -size "${SPRITE_SIZE}x${SPRITE_SIZE}" xc:none "$empty_png"

    local stem png count=0 index
    : > "$montage_list"
    for index in "${!ORDER[@]}"; do
        if [[ $index -ge $MAX_SPRITES ]]; then
            echo "Warning: only the first $MAX_SPRITES portraits fit in the ${COLS}x${ROWS} sheet; skipping ${ORDER[$index]}" >&2
            break
        fi
        stem="${ORDER[$index]}"
        png="$PORT_DIR/${stem}.png"
        if [[ -f "$png" ]]; then
            validate_portrait_size "$png"
            printf '%s\n' "$(to_windows_path "$png")" >> "$montage_list"
            count=$((count + 1))
        else
            printf '%s\n' "$(to_windows_path "$empty_png")" >> "$montage_list"
        fi
    done

    while [[ $(wc -l < "$montage_list" | tr -d ' ') -lt $MAX_SPRITES ]]; do
        printf '%s\n' "$(to_windows_path "$empty_png")" >> "$montage_list"
    done

    mkdir -p "$(dirname "$OUTPUT_FILE")"
    magick montage -background none -tile "${COLS}x${ROWS}" \
        -geometry "${SPRITE_SIZE}x${SPRITE_SIZE}+0+0" \
        @"$montage_list" "$(to_windows_path "$OUTPUT_FILE")"

    echo "Wrote $OUTPUT_FILE ($count portraits, ${COLS}x${ROWS} grid)"
    rm -rf "$temp_dir"
}

discover_portraits

if [[ ${#PORTRAITS_DISCOVERED[@]} -eq 0 ]]; then
    echo "Error: no ${PORTRAIT_PREFIX}*.png or ${PORTRAIT_PREFIX}*.pdn files found in $PORT_DIR" >&2
    exit 1
fi

if [[ $INIT_ORDER -eq 1 ]]; then
    ORDER=("${PORTRAITS_DISCOVERED[@]}")
    write_order_file
    echo "Initialized $ORDER_FILE with ${#ORDER[@]} portraits (alphabetical)."
else
    read_order_file
    if [[ ${#ORDER[@]} -eq 0 ]]; then
        ORDER=("${PORTRAITS_DISCOVERED[@]}")
        write_order_file
        echo "Created $ORDER_FILE with ${#ORDER[@]} portraits (alphabetical)."
    else
        sync_order || true
    fi
fi

print_index_map
print_sdl2w_asset_lines

if [[ $LIST_ONLY -eq 1 ]]; then
    exit 0
fi

build_spritesheet
