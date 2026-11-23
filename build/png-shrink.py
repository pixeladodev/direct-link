#!/usr/bin/env python3
"""png-shrink.py IN.png OUT.png: the same pixels, smaller (the preview writes plain rows; this picks a PNG filter per
row and deflates at level 9). For the README's screenshots. 8-bit RGB or RGBA, non-interlaced."""
import struct, sys, zlib

def chunks(data):
    i = 8
    while i < len(data):
        n, = struct.unpack(">I", data[i:i + 4]); t = data[i + 4:i + 8]
        yield t, data[i + 8:i + 8 + n]; i += 12 + n

def unfilter(raw, w, h, bpp):
    stride = w * bpp; out = bytearray(); prev = bytearray(stride); i = 0
    for _ in range(h):
        f = raw[i]; line = bytearray(raw[i + 1:i + 1 + stride]); i += 1 + stride
        for x in range(stride):
            a = line[x - bpp] if x >= bpp else 0; b = prev[x]; c = prev[x - bpp] if x >= bpp else 0
            if f == 1: line[x] = (line[x] + a) & 255
            elif f == 2: line[x] = (line[x] + b) & 255
            elif f == 3: line[x] = (line[x] + (a + b) // 2) & 255
            elif f == 4:
                p = a + b - c; pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                line[x] = (line[x] + (a if pa <= pb and pa <= pc else b if pb <= pc else c)) & 255
        out += line; prev = line
    return out

def refilter(px, w, h, bpp):
    stride = w * bpp; out = bytearray(); prev = bytes(stride)
    for y in range(h):
        line = px[y * stride:(y + 1) * stride]
        sub = bytes((line[x] - (line[x - bpp] if x >= bpp else 0)) & 255 for x in range(stride))
        up = bytes((line[x] - prev[x]) & 255 for x in range(stride))
        best = min(((0, line), (1, sub), (2, up)), key=lambda t: sum(v if v < 128 else 256 - v for v in t[1]))
        out.append(best[0]); out += best[1]; prev = line
    return bytes(out)

data = open(sys.argv[1], "rb").read()
ihdr = b""; idat = b""; other = []
for t, d in chunks(data):
    if t == b"IHDR": ihdr = d
    elif t == b"IDAT": idat += d
w, h, depth, ctype = struct.unpack(">IIBB", ihdr[:10])
bpp = {2: 3, 6: 4}[ctype]
px = unfilter(zlib.decompress(idat), w, h, bpp)
body = zlib.compress(refilter(px, w, h, bpp), 9)
def chunk(t, d): return struct.pack(">I", len(d)) + t + d + struct.pack(">I", zlib.crc32(t + d) & 0xffffffff)
open(sys.argv[2], "wb").write(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", ihdr) + chunk(b"IDAT", body) + chunk(b"IEND", b""))
