#!/usr/bin/env python3
"""Direct Link's mark, drawn pixel by pixel on an 80 x 72 grid in Pixelado's colours (design/tokens.json in Pixelado
   Party), on transparency: two dual-screen handhelds linked by a cyan spark. Each one's top screen shows its
   character (Nilo, who hosts, in the blue one; Lumi, who joins, in the lavender one; assets/characters).
     python3 make_icons.py [OUT_DIR]
   writes into OUT_DIR (default: this folder):
     tile.png        320 x 288  the Apps shelf's picture (the grid 4x)
     logo.png        160 x 144  the README's (2x)
     icon.png         32 x 32   the Store's icon (the grid scaled down)
     android/        Android's: ic_launcher_foreground.png (432 x 432, the mark 3x inside the 66 dp safe zone of an
                     adaptive icon's 108 dp) and play-store.png (512 x 512, on night.900)
   Needs Pillow."""
import os
import sys
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "characters"))
sys.dont_write_bytecode = True
import characters as CH   # noqa: E402  (the characters' maps and colours)

GW, GH = 80, 72


def hexc(h):
    return tuple(int(h[i:i + 2], 16) for i in (1, 3, 5)) + (255,)


INK = {
    "out": hexc("#070B17"),        # night.950: the outline, so it reads on any background
    "hinge": hexc("#5E73B8"),      # night.400
    "glass": hexc("#1A3C99"),      # accent.dark
    "glass_hi": hexc("#45D4FF"),   # accent.glow
    "spark": hexc("#45D4FF"),
    "spark_hi": hexc("#9CF0FF"),
    "core": hexc("#EAF0FF"),
}
# each handheld in its character's colours: shell, its light edge, its dark foot
SHELLS = {
    "nilo": (hexc("#9CBEFF"), hexc("#EAF0FF"), hexc("#3D7DFF")),
    "lumi": (hexc("#C4CCFF"), hexc("#F2F4FF"), hexc("#7C8FE0")),
}


class Grid:
    def __init__(self):
        self.im = Image.new("RGBA", (GW, GH), (0, 0, 0, 0))

    def put(self, x, y, c):
        if 0 <= x < GW and 0 <= y < GH:
            self.im.putpixel((x, y), c)

    def rect(self, x, y, w, h, c):
        for yy in range(y, y + h):
            for xx in range(x, x + w):
                self.put(xx, yy, c)

    def box(self, x, y, w, h, fill, cut=2):
        """a filled box with a 1-px outline and corners cut in `cut` pixel steps"""
        for yy in range(h):
            for xx in range(w):
                cx, cy = min(xx, w - 1 - xx), min(yy, h - 1 - yy)
                if cx + cy < cut:
                    continue
                edge = cx == 0 or cy == 0 or cx + cy == cut
                self.put(x + xx, y + yy, INK["out"] if edge else fill)


def head(g, who, x, y, h):
    """the character's head and shoulders: the sprite's first h rows, at x, y"""
    by = {c[0].lower(): c for c in CH.CAST}
    _, rows, pal = by[who][:3]
    cols = dict(CH.COMMON, **pal)
    for r in range(min(h, len(rows))):
        for c, ch in enumerate(rows[r]):
            if ch != ".":
                g.put(x + c, y + r, hexc(cols[ch]))


def handheld(g, x, y, who, w=32):
    """w x 54: the top half (w x 28) with a screen showing its character, a hinge, the bottom half (w x 24) with a
       screen, the pad and the buttons"""
    shell, hi, lo = SHELLS[who]
    # top half
    g.box(x, y, w, 28, shell)
    g.rect(x + 3, y + 1, w - 6, 1, hi)
    g.rect(x + 2, y + 25, w - 4, 2, lo)
    g.rect(x + 3, y + 3, w - 6, 22, INK["out"])               # the screen's bezel
    g.rect(x + 4, y + 4, w - 8, 20, INK["glass"])
    g.rect(x + 4, y + 4, w - 8, 1, INK["glass_hi"])
    # the character in it, head and shoulders, cut by the screen's bottom edge (the sprite is 22 wide)
    head(g, who, x + (w - 22) // 2, y + 6, 18)
    # the hinge
    g.rect(x + 4, y + 28, w - 8, 2, INK["out"])
    g.rect(x + 6, y + 28, w - 12, 1, INK["hinge"])
    # bottom half
    by = y + 30
    g.box(x, by, w, 24, shell)
    g.rect(x + 3, by + 1, w - 6, 1, hi)
    g.rect(x + 2, by + 21, w - 4, 2, lo)
    sw = 16
    sx = x + (w - sw) // 2
    g.rect(sx, by + 3, sw, 16, INK["out"])
    g.rect(sx + 1, by + 4, sw - 2, 14, INK["glass"])
    g.rect(sx + 1, by + 4, sw - 2, 1, INK["glass_hi"])
    # the pad (a plus) and the four buttons (a diamond of 2 x 2 dots)
    g.rect(x + 2, by + 10, 5, 1, lo); g.rect(x + 4, by + 8, 1, 5, lo)
    g.rect(x + 3, by + 9, 3, 3, lo)
    for (dx, dy) in ((w - 5, 6), (w - 7, 9), (w - 3, 9), (w - 5, 12)):
        g.rect(x + dx, by + dy, 2, 2, lo)


def spark(g, x0, y0, r):
    """a diamond with an even width and height (a 2 x 2 core), its centre between pixels: x0, y0 its top left core
       pixel, r its radius; outlined, cyan with a lighter upper half and a white core. (An odd diamond can't sit in
       the middle of an even gap: one side touched a console, the other didn't.)"""
    for y in range(y0 - r - 1, y0 + r + 3):
        for x in range(x0 - r - 1, x0 + r + 3):
            d = int(abs(x - (x0 + 0.5)) - 0.5) + int(abs(y - (y0 + 0.5)) - 0.5)
            if d == r + 1:
                g.put(x, y, INK["out"])
            elif d <= r:
                c = INK["core"] if d <= 1 else INK["spark_hi"] if y <= y0 else INK["spark"]
                g.put(x, y, c)


def mark():
    g = Grid()
    # the link, drawn first so the consoles sit on it: a 2-px beam from one hinge to the other (rows 37-38, as tall as
    # the hinges; the hinges end 4 px inside each console, so it starts there)
    g.rect(30, 36, 20, 4, INK["out"])
    g.rect(30, 37, 20, 2, INK["spark"])
    # (2 px of air on each side: 2 + 32 + 12 + 32 + 2)
    handheld(g, 2, 9, "nilo")
    handheld(g, 46, 9, "lumi")
    # the spark in the middle of the 12-px gap (columns 34-45: its centre between 39 and 40)
    spark(g, 39, 37, 3)
    return g.im


def place(im, size, k, bg=None):
    out = Image.new("RGBA", size, bg or (0, 0, 0, 0))
    big = im.resize((im.width * k, im.height * k), Image.NEAREST)
    out.alpha_composite(big, ((size[0] - big.width) // 2, (size[1] - big.height) // 2))
    return out


def icon32(im):
    """the Store's 32 x 32: the mark scaled down (box filter)"""
    small = im.resize((32, 29), Image.BOX)
    out = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    out.alpha_composite(small, (0, 2))
    return out


if __name__ == "__main__":
    out = sys.argv[1] if len(sys.argv) > 1 else HERE
    m = mark()
    os.makedirs(os.path.join(out, "android"), exist_ok=True)
    files = {
        "tile.png": place(m, (320, 288), 4),
        "logo.png": place(m, (160, 144), 2),
        "icon.png": icon32(m),
        "android/ic_launcher_foreground.png": place(m, (432, 432), 3),
        "android/play-store.png": place(m, (512, 512), 6, hexc("#0B1124")),
    }
    for n, im in files.items():
        im.save(os.path.join(out, n), optimize=True)
        print(os.path.join(out, n))
