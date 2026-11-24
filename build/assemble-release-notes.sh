#!/bin/bash
# Print the release body: release-notes/<ver>.md, after checking it and that the zip and its SHA256SUMS exist.
set -euo pipefail
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
"$ROOT/build/check-release-notes.sh" >&2
ver=$(sed -n 's/^app_version=//p' "$ROOT/manifest" | head -n 1 | tr -d '\r')
zip="$ROOT/dist/direct-link-${ver}.zip"
sums="$ROOT/dist/SHA256SUMS"
[ -f "$zip" ] || { echo "assemble-release-notes: missing $zip (run release-build first)" >&2; exit 1; }
[ -f "$sums" ] || { echo "assemble-release-notes: missing $sums" >&2; exit 1; }
# (the checksums are in the SHA256SUMS asset; the notes end with the Ko-fi banner, nothing after it)
cat "$ROOT/release-notes/${ver}.md"
