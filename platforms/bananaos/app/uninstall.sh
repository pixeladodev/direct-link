#!/bin/sh
# Disconnect, then delete this app and its Apps tile. The UI asks first.
set -eu
APP=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
case $APP in
    /data/store/apps/direct-link) ;;
    *) echo "refusing to remove $APP" >&2; exit 1 ;;
esac
sh "$APP/wifi-link" down || true
rm -f "/data/store/tiles/Direct Link.sh" "/data/store/tiles/._Direct Link.sh" \
    "/data/store/tiles/.DS_Store" "/data/store/tiles/media/box/Direct Link.png"
rmdir /data/store/tiles/media/box /data/store/tiles/media 2>/dev/null || true
cd /
rm -rf "$APP"
rm -f "/data/store/apps/._direct-link" "/data/store/apps/.DS_Store"
sync 2>/dev/null || true
