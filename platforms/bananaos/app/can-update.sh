#!/bin/sh
# The app's updater asks before it replaces this folder: not while a link runs from it.
APP=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
case $(sh "$APP/status.sh" 2>/dev/null | sed -n 's/^state=//p' | head -n 1) in
    ""|down) exit 0 ;;
esac
echo "Disconnect first"
exit 1
