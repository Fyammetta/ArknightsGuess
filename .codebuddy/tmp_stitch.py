from PIL import Image, ImageSequence

SRC = r"C:\Users\Admin\Pictures\kfQo3nr-64a0XcZ39T3cSck-dc.gif"
OUT = r"C:\Users\Admin\Pictures\kfQo3nr_frames_strip.png"
CANVAS = 256

im = Image.open(SRC)
frames = [f.convert("RGBA").copy() for f in ImageSequence.Iterator(im)]

tiles = []
for fr in frames:
    w, h = fr.size
    scale = min(CANVAS / w, CANVAS / h)
    nw, nh = max(1, int(round(w * scale))), max(1, int(round(h * scale)))
    resized = fr.resize((nw, nh), Image.LANCZOS)
    canvas = Image.new("RGBA", (CANVAS, CANVAS), (0, 0, 0, 0))
    x = (CANVAS - nw) // 2   # 水平居中
    y = CANVAS - nh          # 垂直向下对齐（贴底）
    canvas.paste(resized, (x, y), resized)
    tiles.append(canvas)

total_h = CANVAS * len(tiles)
strip = Image.new("RGBA", (CANVAS, total_h), (0, 0, 0, 0))
for i, t in enumerate(tiles):
    strip.paste(t, (0, i * CANVAS), t)

strip.save(OUT)
print("saved:", OUT, strip.size, strip.mode, "frames:", len(tiles))
