#!/bin/sh
# Start this handheld as the host (10.42.0.1). No --owner: the link stays up
# after this app closes, until Disconnect.
set -eu
APP=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
exec sh "$APP/wifi-link" up host
