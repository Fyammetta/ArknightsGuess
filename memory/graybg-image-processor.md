---
name: graybg-image-processor
description: 立绘 graybg 背景填充处理工具——为透明背景立绘生成带8×8网格渐变的灰调背景
metadata:
  type: project
  originSessionId: current
  modified: 2026-08-05
---

# 立绘 Graybg 背景填充处理

## 用途
为 ArknightsGuess 项目处理立绘图片，将透明背景替换为与角色色调协调的 8×8 像素化渐变灰调背景，统一输出 2048×2048。

参考数据源：`桌面/mrfz_skins_6star_graybg/`（处理后的参考图），`桌面/mrfz_skins_6star/`（原始图）。

## 算法流程

1. **取均值 α**：遍历立绘所有非透明像素，计算 RGB 均值 → 得到基准背景色 `meanColor`
2. **饱和度 -30%**：将 `meanColor` 从 RGB 转 HSL，饱和度 `S × 0.7`
3. **亮度调整**：若 L > 50%，`L × 0.8`；否则 `L × 1.2`（上限 100%）
4. **填充透明区域**：用调整后的颜色（转回 RGB）作为背景基色
5. **向外渐变**：
   - 径向分量：以角色包围盒中心为光源，近角色处亮度 1.5×，远边缘 0.3×
   - 纵向分量：顶部略亮（1.15×），底部略暗（0.65×）
   - 合并 = `radialF × topF`，钳制在 [0.2, 1.5]
6. **8×8 网格均值**：将 2048×2048 画布均匀分为 64 块（256×256/块），每块内**仅对背景像素**（非角色区域）取渐变均值，整块填充该颜色
7. **立绘合成**：将原立绘按 Alpha 通道混合叠加到背景上，**角色像素不改动**
8. **尺寸修正**：立绘居中放置（不拉伸），输出固定 2048×2048 PNG

## 实现工具

**依赖**：Node.js + `sharp`（npm 包）

**核心代码模板**：
```js
const sharp = require('sharp');

// RGB ↔ HSL 转换函数（见完整脚本）
function rgbToHsl(r, g, b) { /* ... */ }
function hslToRgb(h, s, l) { /* ... */ }

async function processGraybg(inputPath, outputPath) {
    // 1. 读取原图
    const orig = await sharp(inputPath).raw().ensureAlpha().toBuffer({ resolveWithObject: true });
    const origPixels = new Uint8Array(orig.data);
    const { width: origW, height: origH } = orig.info;

    // 2. 计算非透明像素 RGB 均值
    let sumR = 0, sumG = 0, sumB = 0, n = 0;
    for (let i = 0; i < origW * origH; i++) {
        const idx = i * 4;
        if (origPixels[idx + 3] > 0) {
            sumR += origPixels[idx]; sumG += origPixels[idx + 1]; sumB += origPixels[idx + 2]; n++;
        }
    }

    // 3. HSL 调整：饱和度 -30%，亮度 ±20%
    let hsl = rgbToHsl(Math.round(sumR/n), Math.round(sumG/n), Math.round(sumB/n));
    hsl.s = Math.max(0, hsl.s * 0.7);
    hsl.l = hsl.l > 50 ? hsl.l * 0.8 : Math.min(100, hsl.l * 1.2);
    const bg = hslToRgb(hsl.h, hsl.s, hsl.l);

    // 4. 输出配置
    const OUT = 2048, GRID = 8, BW = OUT/GRID, BH = OUT/GRID;
    const ox = Math.floor((OUT - origW) / 2);
    const oy = Math.floor((OUT - origH) / 2);

    // 5. 计算连续渐变场
    const gradient = new Float32Array(OUT * OUT);
    const charCX = ox + origW/2, charCY = oy + origH/2;
    for (let y = 0; y < OUT; y++) {
        for (let x = 0; x < OUT; x++) {
            const dist = Math.sqrt((x-charCX)**2 + (y-charCY)**2);
            const radialF = 1.5 - Math.min(1, dist / (OUT*0.7)) * 1.2;
            const topF = 1.15 - (y / OUT) * 0.5;
            gradient[y * OUT + x] = Math.max(0.2, Math.min(1.5, radialF * topF));
        }
    }

    // 6. 每块取背景均值 → 像素化
    const blockColor = Array.from({length: GRID}, () => new Array(GRID));
    for (let gy = 0; gy < GRID; gy++) {
        for (let gx = 0; gx < GRID; gx++) {
            const y0 = gy*BH, y1 = y0+BH, x0 = gx*BW, x1 = x0+BW;
            let gSum = 0, gCount = 0;
            for (let y = y0; y < y1; y++) {
                for (let x = x0; x < x1; x++) {
                    const lx = x - ox, ly = y - oy;
                    const isChar = lx >= 0 && lx < origW && ly >= 0 && ly < origH
                        && origPixels[(ly*origW + lx)*4 + 3] > 128;
                    if (!isChar) { gSum += gradient[y*OUT + x]; gCount++; }
                }
            }
            const avg = gCount > 0 ? gSum/gCount : gradient[Math.floor(y0+BH/2)*OUT + Math.floor(x0+BW/2)];
            blockColor[gy][gx] = {
                r: Math.round(Math.min(255, Math.max(0, bg.r * avg))),
                g: Math.round(Math.min(255, Math.max(0, bg.g * avg))),
                b: Math.round(Math.min(255, Math.max(0, bg.b * avg)))
            };
        }
    }

    // 7. 渲染输出 + Alpha 合成
    const outBuf = Buffer.alloc(OUT * OUT * 4);
    for (let y = 0; y < OUT; y++) {
        const gy = Math.min(Math.floor(y / BH), GRID - 1);
        for (let x = 0; x < OUT; x++) {
            const gx = Math.min(Math.floor(x / BW), GRID - 1);
            const c = blockColor[gy][gx];
            const idx = (y*OUT + x)*4;
            outBuf[idx]=c.r; outBuf[idx+1]=c.g; outBuf[idx+2]=c.b; outBuf[idx+3]=255;
        }
    }
    // Alpha blend 角色像素（不改动角色 RGB，仅在透明处做混合）
    for (let ly = 0; ly < origH; ly++) {
        for (let lx = 0; lx < origW; lx++) {
            const oi = (ly*origW + lx)*4;
            const a = origPixels[oi + 3];
            if (a > 0) {
                const ox2 = ox+lx, oy2 = oy+ly;
                if (ox2>=0 && ox2<OUT && oy2>=0 && oy2<OUT) {
                    const oi2 = (oy2*OUT + ox2)*4;
                    const fa = a/255;
                    outBuf[oi2]   = Math.round(origPixels[oi]*fa   + outBuf[oi2]*(1-fa));
                    outBuf[oi2+1] = Math.round(origPixels[oi+1]*fa + outBuf[oi2+1]*(1-fa));
                    outBuf[oi2+2] = Math.round(origPixels[oi+2]*fa + outBuf[oi2+2]*(1-fa));
                }
            }
        }
    }
    await sharp(outBuf, { raw: { width: OUT, height: OUT, channels: 4 } })
        .png({ compressionLevel: 9 }).toFile(outputPath);
}
```

## 快速使用
```bash
# 安装依赖（一次性）
mkdir temp_imgproc && cd temp_imgproc && npm init -y && npm install sharp

# 运行处理
node -e "
const sharp = require('sharp');
// ... 粘贴上述 processGraybg 函数 ...
processGraybg('输入.png', '输出_graybg.png').catch(e => console.error(e));
"
```

## 参数调校

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `OUT` | 2048 | 输出画布尺寸 |
| `GRID` | 8 | 网格密度（8×8=64块） |
| 径向峰值 | 1.5 | 角色中心亮度倍率 |
| 径向谷值 | 0.3 | 远边缘亮度倍率 |
| 纵向峰值 | 1.15 | 顶部亮度倍率 |
| 纵向谷值 | 0.65 | 底部亮度倍率 |
| `maxDist` 除数 | 0.7 | 越大=渐变过渡越柔和 |

## 与 Skin 2048×2048 规整的关系
- [[operators-json-workflow]] 中的 2048×2048 规整是**纯缩放+居中**，保留透明背景
- 本工具是在其上**额外叠加灰调像素化背景**，用于猜测游戏等需要完整背景的场景
- 处理顺序：先规整到 2048 → 再 graybg 填充

## 验证方式
- 立绘不改动验证：遍历角色像素，对比处理前后 RGB 差异，opaque pixels avg diff 应 < 0.1
- 背景覆盖验证：处理后的图像应无透明像素残留
- 网格验证：每块内背景像素颜色应完全一致（block uniformity）
