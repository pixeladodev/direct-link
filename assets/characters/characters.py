#!/usr/bin/env python3
"""Pixelado's characters: chibi sprites on a 22 x 24 grid, drawn as text maps in the design system's colours.
     python3 characters.py [OUT_DIR]        -> sheet.png, idle.gif, <name>.png (8x, on the night background; Pillow)
     python3 characters.py --c FILE NAME..  -> a C header with those characters' maps and palettes (core/cast_sprites.h;
                                               no Pillow needed)
   The idle: a 1-px bob and a blink, each character a beat apart (core/cast.c does the same on the handheld)."""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = HERE
FONT = os.path.join(HERE, "../../../BananaLinux/ui/assets/rnds/fonts/PixelifySans-Medium.ttf")

# shared colours
COMMON = {
    "K": "#070B17",   # outline (night.950)
    "S": "#F6D9C0", "s": "#D9B39A",   # skin, its shade
    "W": "#EAF0FF",   # eye shine
    "B": "#FF8FA3",   # blush
    "M": "#B5566A",   # mouth
}

NILO = [  # male: short spiky hair, cyan headphones (band over the head), hoodie
    ".....KKAAAAAAAAKK.....",
    "....KAKKKKKKKKKKAK....",
    "...KAKKHHHHHHHHKKAK...",
    "..KAKHHLLLHHHHHHHKAK..",
    ".KAKHHLLHHHHHHHHHHKAK.",
    ".KAKHHHHHHHHHHHHHHKAK.",
    "KAKHHHHHHHHHHHHHHHHKAK",
    "KAKHHHHhHHHHHhHHHHHKAK",
    "KAKHHHhSShHHhSSShHHKAK",
    "aAKHhSSSShSSSSSSShHKAa",
    "aAKHSSSSSSSSSSSSSSHKAa",
    "aAKSSSKKSSSSSSKKSSSKAa",
    "aAKSSSWESSSSSSWESSSKAa",
    "aAKSSBEESSSSSSEEBSSKAa",
    "..KSSSSSSSMMSSSSSSSK..",
    "...KsSSSSSSSSSSSSsK...",
    "....KKKKKsSSsKKKKK....",
    "....KCCCCcAAcCCCCK....",
    "...KCCCCCCAACCCCCCK...",
    "...KCKCCCCCCCCCCKCK...",
    "...KSKCCCCCCCCCCKSK...",
    "....KKccccccccccKK....",
    "......KNNNKKNNNK......",
    "......KKKK..KKKK......",
]

LUMI = [  # female: a bob with a star clip, a short jacket and skirt
    ".......KKKKKKKK.......",
    ".....KKHHHHHHHHKK.....",
    "....KHHLLLHHHHYYHK....",
    "...KHHLLHHHHHHHYYHK...",
    "...KHHHHHHHHHHHHHHK...",
    "..KHHHHHHHHHHHHHHHHK..",
    "..KHHHHHHHHHHHHHHHHK..",
    "..KHHHHhHHHHHHhHHHHK..",
    "..KHhSSSSSSSSSSSShHK..",
    "..KHhSSSSSSSSSSSShHK..",
    "..KHhSKKSSSSSSKKShHK..",
    "..KHhSWESSSSSSWEShHK..",
    "..KHhBEESSSSSSEEBhHK..",
    "..KHhSSSSSMMSSSSShHK..",
    "..KHHKsSSSSSSSSsKHHK..",
    "...KHKKKKsSSsKKKKHK...",
    "....KCCCCAAAACCCCK....",
    "...KCCCCCCWWCCCCCCK...",
    "...KCKCCCCCCCCCCKCK...",
    "...KSKCCCCCCCCCCKSK...",
    "...KccccccccccccccK...",
    "....KKKKKKKKKKKKKK....",
    "......KSSK..KSSK......",
    "......KKKK..KKKK......",
]

TAVO = [  # male: a beanie with a pompom, messy fringe, a jacket
    ".........KAAK.........",
    "........KAAAaK........",
    ".....KKKKKKKKKKKK.....",
    "....KPPPPPPPPPPPPK....",
    "...KPPLLPPPPPPPPPPK...",
    "..KPPPPPPPPPPPPPPPPK..",
    "..KPPPPPPPPPPPPPPPPK..",
    "..KQQQQQQQQQQQQQQQQK..",
    "..KHHhHHhHHHhHHhHHHK..",
    "..KHhSShSSSSSShSShHK..",
    "..KHSSSSSSSSSSSSSSHK..",
    "..KSSSKKSSSSSSKKSSSK..",
    "..KSSSWESSSSSSWESSSK..",
    "..KSSBEESSSSSSEEBSSK..",
    "..KSSSSSSSMMSSSSSSSK..",
    "...KsSSSSSSSSSSSSsK...",
    "....KKKKKsSSsKKKKK....",
    "....KCCCCKWWKCCCCK....",
    "...KCCCCCKWWKCCCCCK...",
    "...KCKCCCKWWKCCCKCK...",
    "...KSKCCCKWWKCCCKSK...",
    "....KKccccccccccKK....",
    "......KNNNKKNNNK......",
    "......KKKK..KKKK......",
    ]

KIRA = [  # female: long hair swept to one side, a high ponytail on the right with a red tie, a long coat
    ".......KKKKKKKK.......",
    ".....KKHHHHHHHHKK.KKK.",
    "....KHHLLLHHHHHHHKLHHK",
    "...KHHLLHHHHHHHHHHKHhK",
    "...KHHHHHHHHHHHHHHAAK.",
    "..KHHHHHHHHHHHHHHHHKhK",
    "..KHHHHHHHHHHHHHHHHKhK",
    "..KHHHHHHHHHHhhSShHKhK",
    "..KHHHHHHhSSSSSSShHKhK",
    "..KHHHHhSSSSSSSSShHKhK",
    "..KHHhKKSSSSSSKKShHKhK",
    "..KHHSWESSSSSSWEShHKhK",
    "..KHHBEESSSSSSEEBhHKhK",
    "..KHhSSSSSMMSSSSShHKhK",
    "..KHHKsSSSSSSSSsKHHKhK",
    "..KHHKKKKsSSsKKKKHHKK.",
    "..KHKCCCCAAAACCCCKHK..",
    "..KHKCCCCCCCCCCCCKHK..",
    "...KCKCCCCCCCCCCKCK...",
    "...KSKCCCCCCCCCCKSK...",
    "...KCCCCCCCCCCCCCCK...",
    "....KKKKKKKKKKKKKK....",
    "......KNNK..KNNK......",
    "......KKKK..KKKK......",
]

PIXO = [  # male: Pixelado Dev's own: messy black hair with a blue sheen, calm blue eyes, a black hoodie (full body)
    "......KK..KKKK.KK.....",
    ".....KHHKKHHHHKHHK....",
    "....KHHHHHHHHHHHHHK...",
    "...KHHHLLHHHHHHHHHHK..",
    "..KHHHLLHHHHHHHLHHHHK.",
    ".KHHHHHHHHHHHHHHHHHHK.",
    "KHHHHHHHHLHHHHHHHHHHHK",
    ".KHHHHhHHHHhHHHHhHHHK.",
    "..KHHhSHHHhSShHHhSHK..",
    "..KHhSSShHSSSShHSSHK..",
    "..KHSSKKSSSSSSKKSSHK..",
    "..KhSSWESSSSSSWESShK..",
    "..KSSSEESSSSSSEESSSK..",
    "..KSSSSSSSssSSSSSSSK..",
    "...KsSSSSSSSSSSSSsK...",
    "...KCKKKKsSSsKKKKCK...",
    "...KCCCCcKSSKcCCCCK...",
    "...KCCCCCACCACCCCCK...",
    "...KCKCCCACCACCCKCK...",
    "...KSKCCCCCCCCCCKSK...",
    "....KKccccccccccKK....",
    "......KNNNKKNNNK......",
    "......KKKK..KKKK......",
    "......................",
]

PIXO_ABOUT = [  # the same, from the chest up, holding a handheld (the About screens)
    "......KK..KKKK.KK.....",
    ".....KHHKKHHHHKHHK....",
    "....KHHHHHHHHHHHHHK...",
    "...KHHHLLHHHHHHHHHHK..",
    "..KHHHLLHHHHHHHLHHHHK.",
    ".KHHHHHHHHHHHHHHHHHHK.",
    "KHHHHHHHHLHHHHHHHHHHHK",
    ".KHHHHhHHHHhHHHHhHHHK.",
    "..KHHhSHHHhSShHHhSHK..",
    "..KHhSSShHSSSShHSSHK..",
    "..KHSSKKSSSSSSKKSSHK..",
    "..KhSSWESSSSSSWESShK..",
    "..KSSSEESSSSSSEESSSK..",
    "..KSSSSSSSssSSSSSSSK..",
    "...KsSSSSSSSSSSSSsK...",
    "...KCKKKKsSSsKKKKCK...",
    "..KCCCCCcKSSKcCCCCCK..",
    "..KCCCCCAccccACCCCCK..",
    "..KCCCCCACCCCACCCCCK..",
    "..KCCKKKKKKKKKKKKCCK..",
    "..KCKSGGggggggGGSKCK..",
    "..KCKSGGggggggGGSKCK..",
    "..KKKKKKKKKKKKKKKKKK..",
    "......................",
]

PIXO_INK = {"H": "#1E2640", "h": "#0D111F", "L": "#3D6BD6", "E": "#3D7DFF", "A": "#9AA8D6", "C": "#1C2236",
            "c": "#11151F", "N": "#16214A", "G": "#2B3C7A", "g": "#45D4FF"}

CAST = [
    ("Nilo", NILO, {"H": "#3D7DFF", "h": "#1A3C99", "L": "#9CBEFF", "E": "#45D4FF", "A": "#45D4FF", "a": "#2B9CC8",
                    "C": "#2B3C7A", "c": "#1F2D61", "N": "#16214A"}),
    ("Lumi", LUMI, {"H": "#B8C4FF", "h": "#7C8FE0", "L": "#EAF0FF", "E": "#3D7DFF", "Y": "#FFC24A",
                    "A": "#EAF0FF", "C": "#45D4FF", "c": "#2B9CC8", "N": "#16214A"}),
    ("Tavo", TAVO, {"H": "#2A2440", "h": "#151122", "P": "#3D7DFF", "L": "#9CBEFF", "Q": "#1A3C99",
                    "A": "#FFC24A", "a": "#C98F12", "E": "#5EDB8A", "C": "#5E73B8", "c": "#2B3C7A", "W": "#EAF0FF",
                    "N": "#16214A"}),
    ("Kira", KIRA, {"H": "#7A5CFF", "h": "#4B33B8", "L": "#C2B2FF", "E": "#FFC24A", "A": "#FF4F6E",
                    "C": "#16214A", "c": "#0E1433", "N": "#2B3C7A"}),
    ("Pixo", PIXO, PIXO_INK),
    ("Pixo-about", PIXO_ABOUT, PIXO_INK),
]

W, H = 22, 24


def rgba(h):
    return tuple(int(h[i:i + 2], 16) for i in (1, 3, 5)) + (255,)


def sprite(rows, pal, blink=False, rim=None):
    cols = dict(COMMON, **pal)
    im = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    for y, row in enumerate(rows):
        assert len(row) == W, (y, len(row), row)
        for x, ch in enumerate(row):
            if ch == ".":
                continue
            if blink and ch in "WE":
                # closed eyes: the lower eye row becomes a dark line, the rest skin
                ch = "K" if rows[y + 1][x] not in "WE" and ch == "E" else "S"
            im.putpixel((x, y), rgba(cols[ch]))
    if rim:
        solid = {(x, y) for y in range(H) for x in range(W) if im.getpixel((x, y))[3]}
        for (x, y) in list(solid):
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                q = (x + dx, y + dy)
                if 0 <= q[0] < W and 0 <= q[1] < H and q not in solid:
                    im.putpixel(q, rgba(rim))
    return im


def night(w, h):
    bg = Image.new("RGBA", (w, h), rgba("#0B1124"))
    d = ImageDraw.Draw(bg)
    for y in range(0, h, 4):                     # the design system's dither: two faint dots per 4x4
        for x in range(0, w, 4):
            d.point([(x, y), (x + 2, y + 2)], fill=rgba("#101833"))
    return bg


def c_header(path, names):
    by = {c[0]: c for c in CAST}
    out = ["/* Generated by assets/characters/characters.py --c: do not edit; edit the maps there. */",
           "#ifndef CAST_SPRITES_H", "#define CAST_SPRITES_H", "",
           "#define CAST_W %d" % W, "#define CAST_H %d" % H, ""]
    for n in names:
        _, rows, pal = by[n][:3]
        cols = dict(COMMON, **pal)
        used = sorted({ch for r in rows for ch in r if ch != "."})
        cid = n.lower().replace("-", "_")
        out.append("static const char *const cast_%s_map[CAST_H] = {" % cid)
        out += ['    "%s",' % r for r in rows]
        out.append("};")
        out.append("static const CastInk cast_%s_ink[] = {" % cid)
        out += ["    { '%s', 0x%s }," % (ch, cols[ch][1:].upper()) for ch in used]
        out.append("    { 0, 0 }")
        out.append("};")
        out.append("")
    out.append("#endif")
    open(path, "w").write("\n".join(out) + "\n")
    print(path)


def main():
    from PIL import Image, ImageDraw, ImageFont
    global Image, ImageDraw, ImageFont
    k = 8
    sprites = [(c[0], sprite(c[1], c[2], rim=c[3] if len(c) > 3 else None),
                sprite(c[1], c[2], True, c[3] if len(c) > 3 else None)) for c in CAST]
    font = ImageFont.truetype(FONT, 28) if os.path.exists(FONT) else ImageFont.load_default()
    cw, ch = W * k + 40, H * k + 70
    sheet = night(cw * len(sprites), ch)
    d = ImageDraw.Draw(sheet)
    for i, (n, im, _) in enumerate(sprites):
        big = im.resize((W * k, H * k), Image.NEAREST)
        sheet.paste(big, (i * cw + 20, 16), big)
        tw = d.textlength(n, font=font)
        d.text((i * cw + (cw - tw) / 2, H * k + 26), n, font=font, fill=rgba("#EAF0FF"))
        big.save(os.path.join(OUT, n.lower() + ".png"))
    sheet.save(os.path.join(OUT, "sheet.png"))
    # idle: rest, rest, up 1 px, up 1 px, rest, blink (each character offset by a beat so they don't move in step)
    beats = [(0, 0), (0, 0), (1, 0), (1, 0), (0, 0), (0, 1), (0, 0), (0, 0)]
    frames = []
    k2 = 6
    for f in range(len(beats)):
        fr = night(len(sprites) * (W + 4) * k2, (H + 3) * k2)
        for i, (n, im, bl) in enumerate(sprites):
            up, blink = beats[(f + i * 2) % len(beats)]
            src = bl if blink else im
            big = src.resize((W * k2, H * k2), Image.NEAREST)
            fr.paste(big, ((i * (W + 4) + 2) * k2, (2 - up) * k2), big)
        frames.append(fr.convert("P", palette=Image.ADAPTIVE))
    frames[0].save(os.path.join(OUT, "idle.gif"), save_all=True, append_images=frames[1:], duration=180, loop=0,
                   disposal=2)
    print("ok")


if __name__ == "__main__":
    if len(sys.argv) > 2 and sys.argv[1] == "--c":
        c_header(sys.argv[2], sys.argv[3:])
    else:
        if len(sys.argv) > 1:
            OUT = sys.argv[1]
        main()
