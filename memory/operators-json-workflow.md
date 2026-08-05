---
name: operators-json-workflow
description: Operators.json data enrichment workflow for ArknightsGuess project
metadata: 
  node_type: memory
  type: project
  originSessionId: 00831b0b-194b-443a-9c40-4f8efe7566ac
  modified: 2026-08-04T10:06:12.585Z
---

# Operators.json 数据填充流程

## 文件位置
- **数据文件**: `C:\Users\Admin\Desktop\Operators.json`（UTF-16 编码）
- **资产目录**: `C:\Users\Admin\Desktop\ArknightsGuess\Content\UserInterfaces\Textures\`
- **Skin 原始图片**: `C:\Users\Admin\Desktop\Skin\`

## JSON 结构
```json
{
  "Name": "干员名",
  "SkinTextures": [
    {
      "Texture": "/Game/UserInterfaces/Textures/<资产名>.<资产名>",
      "FootModeMultiplier": 8,
      "FootModeOffset": {"X": 0, "Y": 0}
    }
  ],
  "Info": {
    "性别": "",
    "出身地": "",
    "生日": "",
    "种族": "",
    "身高": "",
    "感染情况": "",
    "职业": ""
  }
}
```

## Texture 路径规则
- 格式: `/Game/UserInterfaces/Textures/<资产名>.<资产名>`
- 资产名与 Skin 文件夹中的 png 文件名（不含扩展名）对应
- 例如: `立绘_隐德来希_1` → `/Game/UserInterfaces/Textures/立绘_隐德来希_1.立绘_隐德来希_1`

## 数据来源
- **PRTS wiki** (`prts.wiki`) — 主要数据源，但 fetch 可能被企业策略拦截
- **萌娘百科** (`moegirl.org.cn`) — 备选，同样可能被拦截
- **百度百科** (`baike.baidu.com`) — 作为最后备选
- **WebSearch** — 当直接 fetch 不可用时，通过搜索获取信息片段

## 编码
- 输出文件使用 **UTF-16** 编码（与原始文件一致）
- 验证时可同时输出 UTF-8 版本以便阅读

## 处理流程
1. 先读取现有 `Operators.json` 了解已有条目
2. 核对 `Textures` 文件夹中所有 `.uasset` 文件
3. 确认哪些干员已录入、哪些缺失、哪些皮肤未补全
4. 逐个搜索/抓取干员档案信息（7 个 Info 字段）
5. 生成完整 JSON，覆盖写入 `Operators.json`

## Skin 图片 2048×2048 规整（前置步骤）
新增干员图片放入 `C:\Users\Admin\Desktop\Skin\` 后，先执行画布规整：
- 目标：2048×2048 正方形画布
- **不裁切**，等比缩放（fit = min(2048/w, 2048/h)）
- 缩放后图像在透明画布上**居中**放置
- 覆盖原图（.png）
- 使用 Pillow (PIL)，脚本：`Image.new('RGBA', (2048, 2048))` → `resize(LANCZOS)` → `paste(居中)` → `save`

## 脚部特写 512×512 裁剪
从 `Skin/` → `Skin_Foot/`（`C:\Users\Admin\Desktop\Skin_Foot\`），同名输出 512×512：
- 默认策略：取底部 640px 居中正方形 → 缩放至 512×512
- **已知问题**：部分干员在画布中的站位偏移，统一底部裁剪可能导致脚不在画面中心或裁切位置不对
- 需要时可针对特定干员/皮肤手动调整裁剪区域（偏移 left/upper）
- 输出 PNG，同名覆盖

## 后续任务
博士需要补充所有**六星干员**的数据。届时需要：
- 先将新图片放入 Skin 文件夹，执行 2048×2048 规整
- 确认 Textures 文件夹中有哪些六星干员的资产
- 批量从 PRTS 获取档案数据
- 统一格式填充到 Operators.json
