#!/usr/bin/env bash
# Regenerate the README's images. Requires ffmpeg and a built make_docs.
#
#   cmake -S . -B build -DSVGICONENGINE_BUILD_TOOLS=ON
#   cmake --build build --target make_docs SvgIconEngineTest
#   ./tools/make_docs.sh build
set -euo pipefail

BUILD="${1:-build}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SVGS="$ROOT/test/svgs"
DOCS="$ROOT/docs"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

mkdir -p "$DOCS"

echo "→ rendering frames"
QT_QPA_PLATFORM=offscreen "$BUILD/tools/make_docs" "$SVGS" "$TMP"

# One shared palette per clip keeps the GIFs small and free of dither crawl.
gif() {
    local name="$1" fps="$2"
    ffmpeg -y -loglevel error -framerate "$fps" -i "$TMP/$name/f%04d.png" \
        -vf "fps=$fps,scale=iw:ih:flags=lanczos,palettegen=stats_mode=diff" "$TMP/$name.png"
    ffmpeg -y -loglevel error -framerate "$fps" -i "$TMP/$name/f%04d.png" -i "$TMP/$name.png" \
        -lavfi "fps=$fps,scale=iw:ih:flags=lanczos[x];[x][1:v]paletteuse=dither=bayer:bayer_scale=3" \
        -loop 0 "$DOCS/$name.gif"
    echo "  $DOCS/$name.gif  ($(du -h "$DOCS/$name.gif" | cut -f1))"
}

echo "→ assembling gifs"
gif hover 50
gif press 50
gif theme 25

echo "→ showcase stills"
QT_QPA_PLATFORM=offscreen "$BUILD/test/SvgIconEngineTest" --shot "$DOCS/showcase.png" >/dev/null
echo "  $DOCS/showcase.png and $DOCS/showcase-light.png"
