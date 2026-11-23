#!/bin/sh
# What the on-screen app reads. wifi-link prints state/role/ip; version is ours.
set -eu
APP=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
ver=$(sed -n 's/^app_version=//p' "$APP/manifest" | head -n 1 | tr -d '\r')
sh "$APP/wifi-link" status || true
echo "version=${ver:-}"
