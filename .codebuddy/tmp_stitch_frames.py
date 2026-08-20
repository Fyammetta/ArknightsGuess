from PIL import Image, ImageSequence

SRC = r"C:\Users\Admin\Pictures\kfQo3nr-64a0XcZ39T3cSck-dc.gif"
OUT = r"C:\Users\Admin\Pictures\kfQo3nr_frames_strip.png"
CANVAS = 256  # 画布 256x256
ANCHOR = (CANVAS / 2, CANVAS)  # 画布底部中心 (128, 256)

im = Image.open(SRC)
frames = [f.convert("RGBA").copy() for f in ImageSequence.Iterator(im)]
print("nframes:", len(frames))

tiles = []
for fr in frames:
    w, h = fr.size
    scale = min(CANVAS / w, CANVAS / h)
    nw, nh = max(1, int(round(w * scale))), max(1, int(round(h * scale)))
    resized = fr.resize((nw, nh), Image.LANCZOS)

    # 内容 bbox（在缩放后图片坐标系）
    alpha = resized.split()[-1]
    bbox = alpha.getbbox()
    if not bbox:
        bbox = (0, 0, nw, nh)
    l, t, r, b = bbox
    cx = (l + r) / 2.0      # 内容底部中心 x
    by = float(b)           # 内容底边 y

    # 锚点对齐：内容底部中心 -> 画布底部中心
    px = int(round(ANCHOR[0] - cx))
    py = int(round(ANCHOR[1] - by))

    canvas = Image.new("RGBA", (CANVAS, CANVAS), (0, 0, 0, 0))
    canvas.paste(resized, (px, py), resized)
    tiles.append(canvas)

total_h = CANVAS * len(tiles)
strip = Image.new("RGBA", (CANVAS, total_h), (0, 0, 0, 0))
for i, t in enumerate(tiles):
    strip.paste(t, (0, i * CANVAS), t)

strip.save(OUT)
print("saved:", OUT, strip.size, strip.mode)

# 验证：打印每帧内容底部中心在画布上的位置，应都接近 (128, 256)
print("frame | anchor_on_canvas")
for i, t in enumerate(tiles):
    a = t.split()[-1]
    bb = a.getbbox()
    if bb:
        l, t2, r, b2 = bb
        print(f"{i:2d} | center=({(l+r)/2:.1f},{b2}) bbox=({l},{t2},{r},{b2})")
