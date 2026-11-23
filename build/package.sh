#!/bin/sh
# Build the on-screen app and zip it.
# The zip root is direct-link/, Direct Link.sh and INSTALL.txt. No store/ tree.
#   ./build/package.sh
# The version in the zip name is app_version from manifest (at the root).
# CC: an aarch64 compiler whose sysroot has libdrm. Otherwise the bananads-build:1 image is used.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
ver=$(sed -n 's/^app_version=//p' "$ROOT/manifest" | head -n 1 | tr -d '\r')
[ -n "$ver" ] || { echo "manifest needs app_version" >&2; exit 1; }

# the pixel-ui library: the vendored copy, or a live working copy (PIXEL_UI_DIR=../pixel-ui)
UI=$(CDPATH= cd -- "${PIXEL_UI_DIR:-$ROOT/vendor/pixel-ui}" && pwd) || { echo "no pixel-ui at ${PIXEL_UI_DIR:-vendor/pixel-ui}" >&2; exit 1; }

STAGE=$(mktemp -d)
trap 'rm -rf "$STAGE"' EXIT
APP=$STAGE/direct-link
mkdir -p "$APP/ui" "$ROOT/dist"

DRM_INC=${DRM_INC:-}
if [ -n "${CC:-}" ]; then
    if [ -z "$DRM_INC" ]; then
        SYS=$("$CC" -print-sysroot 2>/dev/null || true)
        [ -n "$SYS" ] && [ -d "$SYS/usr/include/libdrm" ] && DRM_INC="$SYS/usr/include/libdrm"
    fi
    (cd "$ROOT" && "$CC" -O2 -Wall -Wextra ${DRM_INC:+-I"$DRM_INC"} -Icore -I"$UI/include" -I"$UI/third_party" -o "$APP/ui/direct-link" \
        core/view.c "$UI/src/cast.c" "$UI/src/pixel.c" "$UI/src/stb_impl.c" frontends/linux-kms/main.c \
        "$UI/linux-kms/updater.c" "$UI/linux-kms/pixel_sound.c" -ldrm -lm -ldl -lpthread)
elif docker image inspect bananads-build:1 >/dev/null 2>&1; then
    # (a pixel-ui outside the app is mounted read-only beside it)
    docker run --rm --platform linux/amd64 -v "$ROOT:/src:ro" -v "$UI:/pixel-ui:ro" -v "$APP/ui:/out" -w /src bananads-build:1 \
        /workspace/bananads-lan-test/aarch64--glibc--stable-2025.08-1/bin/aarch64-buildroot-linux-gnu-gcc \
        -O2 -Wall -Wextra \
        -I/workspace/bananads-lan-test/aarch64--glibc--stable-2025.08-1/aarch64-buildroot-linux-gnu/sysroot/usr/include/libdrm \
        -Icore -I/pixel-ui/include -I/pixel-ui/third_party -o /out/direct-link \
        core/view.c /pixel-ui/src/cast.c /pixel-ui/src/pixel.c /pixel-ui/src/stb_impl.c frontends/linux-kms/main.c \
        /pixel-ui/linux-kms/updater.c /pixel-ui/linux-kms/pixel_sound.c -ldrm -lm -ldl -lpthread
else
    echo "set CC to an aarch64 gcc with libdrm, or build where bananads-build:1 exists" >&2
    exit 1
fi

cp "$ROOT/platforms/bananaos/app/wifi-link" "$ROOT/platforms/bananaos/app/wifi-guard.sh" "$ROOT/platforms/bananaos/app/host.sh" "$ROOT/platforms/bananaos/app/join.sh" "$ROOT/platforms/bananaos/app/down.sh" \
    "$ROOT/platforms/bananaos/app/status.sh" "$ROOT/platforms/bananaos/app/tile.sh" "$ROOT/platforms/bananaos/app/uninstall.sh" "$ROOT/platforms/bananaos/app/update.py" "$ROOT/platforms/bananaos/app/can-update.sh" \
    "$ROOT/platforms/bananaos/app/app.json" "$ROOT/manifest" "$APP/"
cp "$ROOT/platforms/bananaos/tile/Direct Link.sh" "$STAGE/Direct Link.sh"
# the Apps shelf's picture (tile.sh puts it in store/tiles/media/box)
cp "$ROOT/assets/brand/tile.png" "$APP/tile.png"
cp "$ROOT/INSTALL.txt" "$STAGE/INSTALL.txt"
chmod 755 "$APP/ui/direct-link" "$APP/wifi-link" "$APP/wifi-guard.sh" "$APP/host.sh" "$APP/join.sh" "$APP/down.sh" \
    "$APP/status.sh" "$APP/tile.sh" "$APP/uninstall.sh" "$APP/update.py" "$APP/can-update.sh" \
    "$STAGE/Direct Link.sh"

ZIP=$ROOT/dist/direct-link-$ver.zip
python3 - "$STAGE" "$ZIP" "$ROOT/dist/SHA256SUMS" << 'PY'
import hashlib, os, sys, zipfile
stage, dest, sums = sys.argv[1], sys.argv[2], sys.argv[3]
files = []
for dirpath, _, names in os.walk(stage):
    for name in names:
        full = os.path.join(dirpath, name)
        rel = os.path.relpath(full, stage).replace(os.sep, "/")
        base = os.path.basename(rel)
        if base.startswith("._") or base == ".DS_Store" or "__MACOSX" in rel.split("/"):
            continue
        files.append((rel, full))
files.sort()
with zipfile.ZipFile(dest, "w", zipfile.ZIP_DEFLATED) as z:
    for rel, full in files:
        info = zipfile.ZipInfo(rel)
        info.external_attr = (os.stat(full).st_mode & 0xFFFF) << 16
        info.compress_type = zipfile.ZIP_DEFLATED
        info.date_time = (1980, 1, 1, 0, 0, 0)
        with open(full, "rb") as fh:
            z.writestr(info, fh.read())
h = hashlib.sha256()
with open(dest, "rb") as fh:
    for chunk in iter(lambda: fh.read(1 << 20), b""):
        h.update(chunk)
with open(sums, "w") as fh:
    fh.write("%s  %s\n" % (h.hexdigest(), os.path.basename(dest)))
with zipfile.ZipFile(dest) as z:
    names = z.namelist()
tops = set(n.split("/")[0] for n in names)
want = {"direct-link", "Direct Link.sh", "INSTALL.txt"}
if tops != want or any(n.startswith("store/") or n.startswith("._") or "__MACOSX" in n for n in names):
    sys.stderr.write("zip root is %s\n" % ", ".join(sorted(tops)))
    sys.exit(1)
print(dest)
PY
if command -v sha256sum >/dev/null 2>&1; then
    sha256sum "$ZIP" "$APP/ui/direct-link"
else
    shasum -a 256 "$ZIP" "$APP/ui/direct-link"
fi
