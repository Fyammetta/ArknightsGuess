from PIL import Image, ImageSequence

SRC = r"C:\Users\Admin\Pictures\kfQo3nr-64a0XcZ39T3cSck-dc.gif"
CANVAS = 256

im = Image.open(SRC)
frames = [f.convert("RGBA").copy() for f in ImageSequence.Iterator(im)]

# 记录每一帧：缩放后尺寸、贴在画布上的位置、内容bbox（在画布坐标下）
print("frame | resized | paste_xy | content_bbox_on_canvas")
for i, fr in enumerate(frames):
    w, h = fr.size
    scale = min(CANVAS / w, CANVAS / h)
    nw, nh = max(1, int(round(w * scale))), max(1, int(round(h * scale)))
    resized = fr.resize((nw, nh), Image.LANCZOS)
    x = (CANVAS - nw) // 2
    y = CANVAS - nh

    # 内容（不透明）bbox
    alpha = resized.split()[-1]
    bbox = alpha.getbbox()  # (left, top, right, bottom) 在原图(缩放后)坐标系
    if bbox:
        # 转换为画布坐标
        l, t, r, b = bbox
        cb_bbox = (l + x, t + y, r + x, b + y)
    else:
        cb_bbox = None
    print(f"{i:2d} | {nw}x{nh} | ({x},{y}) | {cb_bbox}")
