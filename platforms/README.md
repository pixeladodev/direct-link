# platforms

- `bananaos/` — `app/`: the scripts BananaOS's Apps shelf runs (`app.json`, `tile.sh`, `host.sh`, `join.sh`,
  `down.sh`, `status.sh`, `uninstall.sh`, `update.py`) and `wifi-link`, which drives the radio (an IBSS cell through
  its own wpa_supplicant, adapted from BananaLinux); `tile/Direct Link.sh`: the shelf entry.

Another system gets its own folder with the same scripts' contract. If it has no Pixelify Sans, its package carries
the fonts in the app's `fonts/` (the UI looks there after the system's folder).
