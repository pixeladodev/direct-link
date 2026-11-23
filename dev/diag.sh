#!/bin/bash
# What Direct Link did on the consoles, from the Mac. Read-only: it starts and stops nothing. While the link is up the
# consoles are off the home Wi-Fi (the Mac can't reach them): run it after Disconnect.
#   ./dev/diag.sh            # each console's status, state file, wlan0, ARP and both logs' last lines
#   ./dev/diag.sh log [N]    # each console's last N lines of wifi-link's log and the app's (what it showed), merged
#   ./dev/diag.sh save       # both, for both consoles, into dist/diag-<time>.txt (to keep with the notes)
#   ./dev/diag.sh watch      # every 2 s, one line per console, until Ctrl-C (only while they're on the home Wi-Fi)
# Consoles: CONSOLES="ip ip", else dev/consoles.local, else the two of the workspace (rgds, rgds-plus in ~/.ssh/config).
set -uo pipefail
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
if [ -z "${CONSOLES:-}" ] && [ -f "$ROOT/dev/consoles.local" ]; then
    CONSOLES=$(grep -v '^[[:space:]]*#' "$ROOT/dev/consoles.local" | tr '\n' ' ')
fi
CONSOLES=${CONSOLES:-rgds rgds-plus}
APP=/data/store/apps/direct-link
LOG=/data/banana/logs/direct-link.log
UILOG=/data/banana/logs/direct-link-ui.log
SSH="ssh -o ConnectTimeout=4 -o BatchMode=yes"

once() {
    for h in $CONSOLES; do
        echo "==================== $h"
        $SSH "$h" "
            echo '--- status'; sh $APP/wifi-link status 2>&1
            echo '--- state file'; cat /run/banana/direct-link/state 2>/dev/null || echo '(none)'
            echo '--- wlan0'; ip -4 addr show wlan0 2>/dev/null | grep inet; iw dev wlan0 info 2>/dev/null | grep -E 'type|ssid|channel'
            echo '--- arp (10.42.0.x)'; grep '^10\.42\.0\.' /proc/net/arp || echo '(none)'
            echo '--- log (wifi-link)'; tail -n 25 $LOG
            echo '--- log (what the app showed)'; tail -n 15 $UILOG 2>/dev/null || echo '(none yet)'
        " 2>&1 || echo "($h: no answer: asleep, or on the link?)"
    done
}

line() {   # one console's status in one line
    $SSH "$1" "sh $APP/wifi-link status 2>/dev/null | grep -E '^(state|role|phase|ip|peers|peer_ips)=' | tr '\n' ' '" 2>/dev/null ||
        echo "(no answer)"
}

watch_() {
    mkdir -p "$ROOT/dist"
    out="$ROOT/dist/diag-$(TZ=UTC date +%Y%m%d-%H%M%S).log"
    echo "watching $CONSOLES every 2 s (Ctrl-C to stop); log: $out"
    while :; do
        t=$(TZ=UTC date +%T)
        for h in $CONSOLES; do printf '%s %-10s %s\n' "$t" "$h" "$(line "$h")"; done | tee -a "$out"
        sleep 2
    done
}

case "${1:-}" in
    watch) watch_ ;;
    log) for h in $CONSOLES; do
             echo "==================== $h"
             $SSH "$h" "{ tail -n ${2:-60} $LOG | grep '^20'; tail -n ${2:-60} $UILOG 2>/dev/null | sed 's/^\(.\{19\}\) /\1 [app] /'; } | sort" 2>&1
         done ;;
    save) mkdir -p "$ROOT/dist"; f="$ROOT/dist/diag-$(TZ=UTC date +%Y%m%d-%H%M%S).txt"
          { once; "$0" log 120; } > "$f" 2>&1; echo "$f" ;;
    ''|once) once ;;
    *) sed -n '2,8s/^# \{0,1\}//p' "$0" >&2; exit 2 ;;
esac
