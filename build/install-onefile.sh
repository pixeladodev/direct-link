#!/bin/sh
# Apps-shelf tile installer (one .sh copied to store/tiles). dev/deploy.sh does not call this.
#   ./build/package.sh && ./build/install-onefile.sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
ZIP=$(ls -t "$ROOT"/dist/direct-link-*.zip 2>/dev/null | head -n 1)
[ -n "$ZIP" ] || { echo "run ./build/package.sh first" >&2; exit 1; }
SUMS="$ROOT/dist/SHA256SUMS"
[ -f "$SUMS" ] || { echo "missing dist/SHA256SUMS (run package.sh)" >&2; exit 1; }
ZIP_SHA=$(awk '{print $1; exit}' "$SUMS")
HDR=$(mktemp)
OUT="$ROOT/dist/Install-Direct-Link.sh"
cat > "$HDR" << 'HDR'
#!/bin/sh
# Install Direct Link on BananaOS: copy this file once into store/tiles, then open it from Apps.
# It unpacks the app into store/apps, adds the real Direct Link tile, and removes this installer.
set -eu
SELF=$0
APPS=${DIRECT_LINK_APPS:-/data/store/apps}
TILES=${DIRECT_LINK_TILES:-/data/store/tiles}
APP=direct-link
TILE='Direct Link.sh'
WANT_SHA=ZIP_SHA_PLACEHOLDER
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
ZIP=$WORK/pkg.zip
tail -n +ARCHIVE_LINE "$SELF" > "$ZIP"
if command -v sha256sum >/dev/null 2>&1; then
    got=$(sha256sum "$ZIP" | awk '{print $1}')
else
    got=$(shasum -a 256 "$ZIP" | awk '{print $1}')
fi
[ "$got" = "$WANT_SHA" ] || exit 1
unzip -oq "$ZIP" -d "$WORK"
DEST="$APPS/$APP"
rm -rf "$DEST"
mkdir -p "$APPS" "$TILES"
cp -a "$WORK/$APP" "$DEST"
cp "$WORK/$TILE" "$TILES/$TILE"
chmod a+x "$DEST/tile.sh" "$DEST/wifi-link" "$DEST/wifi-guard.sh" "$DEST/host.sh" \
    "$DEST/join.sh" "$DEST/down.sh" "$DEST/status.sh" "$DEST/uninstall.sh" \
    "$DEST/update.py" "$DEST/can-update.sh" "$DEST/ui/direct-link" 2>/dev/null || true
if [ -f "$DEST/tile.png" ]; then
    mkdir -p "$TILES/media/box"
    cp "$DEST/tile.png" "$TILES/media/box/Direct Link.png"
fi
sync 2>/dev/null || true
rm -f "$SELF"
exec sh "$TILES/$TILE"
HDR
MARK=$(($(wc -l < "$HDR") + 1))
sed "s/ARCHIVE_LINE/$MARK/; s/ZIP_SHA_PLACEHOLDER/$ZIP_SHA/" "$HDR" > "$OUT"
cat "$ZIP" >> "$OUT"
chmod 755 "$OUT"
rm -f "$HDR"
echo "$OUT"