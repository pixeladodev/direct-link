#!/bin/sh
# Apps shelf tile. bash runs this, so this file needs no +x.
ROOT=${DIRECT_LINK_APPS:-/data/store/apps}
exec sh "$ROOT/direct-link/tile.sh"