#!/bin/sh
# Join the host's link. Address is the first free one in 10.42.0.2-9.
set -eu
APP=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
exec sh "$APP/wifi-link" up join
