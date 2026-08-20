from PIL import Image, ImageSequence

SRC = r"C:\Users\Admin\Pictures\kfQo3nr-64a0XcZ39T3cSck-dc.gif"
im = Image.open(SRC)
frames = [f.convert("RGBA").copy() for f in ImageSequence.Iterator(im)]

print("frame | bbox(orig) | centroid | bottom_row_foot")
for i, fr in enumerate(frames):
    a = fr.split()[-1]
    bb = a.getbbox()
    l, t, r, b = bb
    # 质心（不透明像素加权）
    w, h = fr.size
    px = a.load()
    sx = sy = cnt = 0
    for y in range(h):
        row = a.crop((0, y, w, y + 1))
        rdata = list(row.getdata())
        for x, v in enumerate(rdata):
            if v > 0:
                sx += x
                sy += y
                cnt += 1
    cx, cy = sx / cnt, sy / cnt
    print(f"{i:2d} | ({l},{t},{r},{b}) | ({cx:.1f},{cy:.1f})")
