#!/bin/bash
# Build the on-screen app and the zip the way a release does (run in debian:trixie-slim).
#   docker run --rm --platform linux/amd64 -v "$PWD:/src" -w /src debian:trixie-slim bash build/release-build.sh
set -euo pipefail
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
cd "$ROOT"

if ! command -v curl >/dev/null 2>&1 || ! command -v python3 >/dev/null 2>&1; then
    export DEBIAN_FRONTEND=noninteractive
    apt-get update
    apt-get install -y --no-install-recommends ca-certificates curl xz-utils tar python3 dpkg-dev
fi

BOOTLIN_NAME=aarch64--glibc--stable-2025.08-1
BOOTLIN_SHA256=dfb47eee874eef9e8a7fc042eee4e0a183f444b6bcde6a82fef8f009918389c9
TC=${DIRECT_LINK_BOOTLIN:-/opt/$BOOTLIN_NAME}
if [ ! -x "$TC/bin/aarch64-linux-gcc" ]; then
    mkdir -p /opt
    curl -fsSLo /tmp/bootlin.tar.xz \
        "https://toolchains.bootlin.com/downloads/releases/toolchains/aarch64/tarballs/$BOOTLIN_NAME.tar.xz"
    echo "$BOOTLIN_SHA256  /tmp/bootlin.tar.xz" | sha256sum -c -
    tar -xJf /tmp/bootlin.tar.xz -C /opt
    rm -f /tmp/bootlin.tar.xz
fi
SYS=$TC/aarch64-buildroot-linux-gnu/sysroot

# libdrm is the shared library already on the console. Debian's arm64 packages
# supply the headers and the link-time .so.
DRM_VER=2.4.124-2
if [ ! -f "$SYS/usr/include/libdrm/drm.h" ]; then
    dpkg --add-architecture arm64
    apt-get update
    debs=/tmp/direct-link-debs
    rm -rf "$debs"
    mkdir -p "$debs"
    (cd "$debs" && apt-get download "libdrm2:arm64=${DRM_VER}" "libdrm-dev:arm64=${DRM_VER}")
    for deb in "$debs"/*.deb; do
        dpkg-deb -x "$deb" "$SYS"
    done
    ln -sfn aarch64-linux-gnu/libdrm.so "$SYS/usr/lib/libdrm.so"
fi

export CC="$TC/bin/aarch64-linux-gcc"
export DRM_INC="$SYS/usr/include/libdrm"
sh "$ROOT/build/selftest.sh"
sh "$ROOT/build/package.sh"
sh "$ROOT/build/install-onefile.sh"
cat "$ROOT/dist/SHA256SUMS"
