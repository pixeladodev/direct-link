#!/bin/bash
# Copy this app onto the consoles, over the installed path. Does not commit.
#   CONSOLES="ip ip" ./dev/deploy.sh
#   ./dev/deploy.sh                 # consoles from dev/consoles.local
#   ./dev/deploy.sh -V 0.1.2-dev    # the version the installed app says (default: the manifest's + -dev,
#                                   # so the updater still offers the next release)
# A link that is up keeps running the old wifi-link until it comes down: Disconnect first, then connect again.
set -euo pipefail
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
VER=
while getopts V: o; do
    case $o in
        V) VER=$OPTARG ;;
        *) sed -n '2,8s/^# \{0,1\}//p' "$0" >&2; exit 2 ;;
    esac
done
if [ -z "${CONSOLES:-}" ] && [ -f "$ROOT/dev/consoles.local" ]; then
    CONSOLES=$(grep -v '^[[:space:]]*#' "$ROOT/dev/consoles.local" | tr '\n' ' ')
fi
[ -n "${CONSOLES:-}" ] || { echo "deploy: set CONSOLES or dev/consoles.local" >&2; exit 2; }
KEY=${DIRECT_LINK_KEY:-$HOME/.ssh/bananaos_rgds}

"$ROOT/build/package.sh" >/dev/null
ver=$(sed -n 's/^app_version=//p' "$ROOT/manifest" | head -n 1 | tr -d '\r')
ZIP=$ROOT/dist/direct-link-$ver.zip
[ -f "$ZIP" ] || { echo "deploy: no $ZIP" >&2; exit 1; }
STAGE=$(mktemp -d)
trap 'rm -rf "$STAGE"' EXIT
unzip -q -d "$STAGE" "$ZIP"
VER=${VER:-$ver-dev}
sed -i.bak "s/^app_version=.*/app_version=$VER/" "$STAGE/direct-link/manifest" && rm -f "$STAGE/direct-link/manifest.bak"
echo "version installed: $VER"

one() {
    local ip=$1
    COPYFILE_DISABLE=1 tar -C "$STAGE" -cf - direct-link "Direct Link.sh" | ssh -i "$KEY" -o IdentitiesOnly=yes -o ConnectTimeout=8 -o BatchMode=yes "root@$ip" '
        set -eu
        d=$(mktemp -d)
        tar -C "$d" -xf -
        mkdir -p /data/store/apps/direct-link /data/store/tiles
        tar -C "$d/direct-link" -cf - . | tar -C /data/store/apps/direct-link -xf -
        cp "$d/Direct Link.sh" "/data/store/tiles/Direct Link.sh"
        rm -rf "$d"
        sha256sum /data/store/apps/direct-link/ui/direct-link
        sed -n "s/^app_version=//p" /data/store/apps/direct-link/manifest
        /data/store/apps/direct-link/status.sh
    '
}
pids=
for ip in $CONSOLES; do
    one "$ip" > "$STAGE/out-$ip.txt" 2>&1 &
    pids="$pids $!"
done
st=0
for p in $pids; do wait "$p" || st=1; done
for ip in $CONSOLES; do
    echo "==== $ip ===="
    cat "$STAGE/out-$ip.txt"
done
exit $st
