#!/bin/sh
# Sits on top of BananaOS's wifi script while Direct Link's own wpa_supplicant is alive.
# up/down/scan/restart from Settings would kill that process, so they do nothing.
# Everything else, and those commands once the link's wpa is gone, run the real script
# (copied to $D/wifi.orig before the mount).
D=${BANANA_LINK_DIR:-/run/banana/direct-link}
REAL=$D/wifi.orig
pid=$(cat "$D/wpa.pid" 2>/dev/null || true)
if [ -n "$pid" ] && kill -0 "$pid" 2>/dev/null; then
    case ${1:-} in
        up|down|restart|scan) exit 0 ;;
    esac
fi
[ -f "$REAL" ] || exit 0
exec "$REAL" "$@"
