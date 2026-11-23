#!/bin/sh
# Script checks. No handheld, no radio, no compiler.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT
fail=0
bad() { echo "FAIL $1"; fail=1; }

for s in wifi-link wifi-guard.sh host.sh join.sh down.sh status.sh tile.sh uninstall.sh; do
    sh -n "$ROOT/platforms/bananaos/app/$s" || bad "syntax $s"
done
sh -n "$ROOT/platforms/bananaos/tile/Direct Link.sh" || bad "syntax tile"
grep -q exec "$ROOT/platforms/bananaos/tile/Direct Link.sh" || bad "tile must exec app tile.sh"

grep -q 'store/apps/direct-link' "$ROOT/platforms/bananaos/app/uninstall.sh" || bad "uninstall path"
grep -q 'Direct Link.sh' "$ROOT/platforms/bananaos/app/uninstall.sh" || bad "uninstall tile"
grep -q '10.42.0.1' "$ROOT/platforms/bananaos/app/wifi-link" || bad "host address"
grep -q '10.42.0.$n' "$ROOT/platforms/bananaos/app/wifi-link" || bad "player range"
grep -q '/run/banana}/direct-link' "$ROOT/platforms/bananaos/app/wifi-link" || bad "state dir"
grep -q 'direct-link.conf' "$ROOT/platforms/bananaos/app/wifi-link" || bad "conf path"
grep -q 'MAX)' "$ROOT/platforms/bananaos/app/wifi-link" || bad "max from conf"
grep '^exec' "$ROOT/platforms/bananaos/app/host.sh" | grep -q -- '--owner' && bad "host passes --owner"
grep '^exec' "$ROOT/platforms/bananaos/app/join.sh" | grep -q -- '--owner' && bad "join passes --owner"

export BANANA_LINK_DIR=$T/run BANANA_LINK_LOG=$T/link.log BANANA_LINK_CONF=$T/direct-link.conf
printf 'CHANNEL=6\nMAX=90\n' > "$BANANA_LINK_CONF"
ver=$(sed -n 's/^app_version=//p' "$ROOT/manifest" | head -n 1 | tr -d '\r')
[ -n "$ver" ] || bad "manifest app_version"
# (as installed: the app's scripts with the manifest beside them)
mkdir -p "$T/app" && cp -R "$ROOT/platforms/bananaos/app/." "$T/app/" && cp "$ROOT/manifest" "$T/app/manifest"
out=$(sh "$T/app/status.sh")
printf '%s\n' "$out" | grep -qx 'state=down' || bad "status down: $out"
printf '%s\n' "$out" | grep -qx "version=$ver" || bad "version: $out"

if sh "$ROOT/platforms/bananaos/app/uninstall.sh" >/dev/null 2>"$T/unerr"; then
    bad "uninstall ran outside the store"
else
    grep -q refusing "$T/unerr" || bad "uninstall message"
fi

# wifi= line is saved, forced off, and put back. The other lines stay.
printf 'language=en\nwifi="on"\nssh=off\n' > "$T/settings.conf"
export BANANA_SETTINGS=$T/settings.conf BANANA_LINK_LOG=$T/link.log
mkdir -p "$T/run"
sh "$ROOT/platforms/bananaos/app/wifi-link" _setting save
sh "$ROOT/platforms/bananaos/app/wifi-link" _setting off
grep -qx 'wifi=off' "$T/settings.conf" || bad "wifi was not forced off"
grep -qx 'language=en' "$T/settings.conf" || bad "settings lost a line"
grep -qx 'ssh=off' "$T/settings.conf" || bad "ssh line lost"
sh "$ROOT/platforms/bananaos/app/wifi-link" _setting restore
grep -qx 'wifi="on"' "$T/settings.conf" || bad "wifi setting was not restored"
grep -qx 'language=en' "$T/settings.conf" || bad "language lost on restore"

# No wifi= line: off adds one, restore removes it.
printf 'ssh=off\n' > "$T/settings.conf"
sh "$ROOT/platforms/bananaos/app/wifi-link" _setting save
sh "$ROOT/platforms/bananaos/app/wifi-link" _setting off
grep -qx 'wifi=off' "$T/settings.conf" || bad "wifi=off was not added"
sh "$ROOT/platforms/bananaos/app/wifi-link" _setting restore
grep -q '^wifi=' "$T/settings.conf" && bad "added wifi line stayed"
grep -qx 'ssh=off' "$T/settings.conf" || bad "ssh lost when removing wifi"

# status prints home_wifi while the supervisor pid is alive.
printf '%s\n' "$$" > "$T/run/super.pid"
printf 'state=up\nrole=host\nhome_wifi=on\n' > "$T/run/state"
out=$(sh "$ROOT/platforms/bananaos/app/wifi-link" status)
printf '%s\n' "$out" | grep -qx 'home_wifi=on' || bad "status hid home_wifi: $out"
rm -f "$T/run/super.pid" "$T/run/state"

# The guard ignores up/down/scan/restart while the link's wpa pid is alive, and runs the real script otherwise.
printf '#!/bin/sh\nprintf "%%s\\n" "$1" >> "%s/called"\n' "$T" > "$T/run/wifi.orig"
chmod 755 "$T/run/wifi.orig"
sleep 30 &
alive=$!
printf '%s\n' "$alive" > "$T/run/wpa.pid"
rm -f "$T/called"
BANANA_LINK_DIR=$T/run sh "$ROOT/platforms/bananaos/app/wifi-guard.sh" down
[ ! -f "$T/called" ] || bad "guard ran down while the link was up"
BANANA_LINK_DIR=$T/run sh "$ROOT/platforms/bananaos/app/wifi-guard.sh" status
grep -qx status "$T/called" || bad "guard blocked status"
kill "$alive" 2>/dev/null || true
wait "$alive" 2>/dev/null || true
printf '%s\n' "$alive" > "$T/run/wpa.pid"
BANANA_LINK_DIR=$T/run sh "$ROOT/platforms/bananaos/app/wifi-guard.sh" down
grep -qx down "$T/called" || bad "guard blocked down after the link"

sh -n "$ROOT/platforms/bananaos/app/can-update.sh" || bad "syntax can-update.sh"
if command -v python3 >/dev/null 2>&1; then
    python3 "$ROOT/build/update-selftest.py" >/dev/null || bad "updater"
else
    echo "selftest: no python3, the updater is not checked"
fi

[ "$fail" = 0 ] && echo "selftest ok" || { echo "selftest failed"; exit 1; }