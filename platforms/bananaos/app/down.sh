#!/bin/sh
# Drop the link and bring the home Wi-Fi back.
set -eu
APP=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
exec sh "$APP/wifi-link" down
