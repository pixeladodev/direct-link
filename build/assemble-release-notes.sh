#!/bin/bash
# Print the release body: release-notes/<ver>.md plus the zip's checksum and the source.
set -euo pipefail
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
"$ROOT/build/check-release-notes.sh" >&2
ver=$(sed -n 's/^app_version=//p' "$ROOT/manifest" | head -n 1 | tr -d '\r')
zip="$ROOT/dist/direct-link-${ver}.zip"
sums="$ROOT/dist/SHA256SUMS"
[ -f "$zip" ] || { echo "assemble-release-notes: missing $zip (run release-build first)" >&2; exit 1; }
[ -f "$sums" ] || { echo "assemble-release-notes: missing $sums" >&2; exit 1; }
zip_sha=$(awk '{print $1; exit}' "$sums")
cat "$ROOT/release-notes/${ver}.md"
echo
echo '---'
echo
echo '## Checksum and source (MIT)'
echo
printf 'SHA256 of `direct-link-%s.zip`: `%s`\n\n' "$ver" "$zip_sha"
echo 'Built by `build/release-build.sh` from this tag. `platforms/bananaos/app/wifi-link` is adapted from BananaLinux; see `LICENSE` and `NOTICE`.'
