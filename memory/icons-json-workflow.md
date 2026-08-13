---
name: icons-json-workflow
description: Icons.json (FOperatorIconRow data table) generation workflow with PRTS launch-order data
metadata: 
  node_type: memory
  type: project
---

# Icons.json 数据表填充流程

## 文件位置
- **输出**: `D:\ArknightsGuess\Content\Datas\Icons.json`（UTF-8 with BOM，供 `DT_OperatorIconData.uasset` reimport）
- **图标资产**: `Content\UserInterfaces\Textures\Icons\头像_<干员名>.uasset`

## JSON 结构（FOperatorIconRow）
```json
[
  {
    "Name": "能天使",
    "Icon": "/Game/UserInterfaces/Textures/Icons/头像_能天使.头像_能天使"
  }
]
```
- 行名 = 干员名 = 资产 BaseName 去掉 `头像_` 前缀
- 排序 = （上线时间升序, 干员id编号升序）

## PRTS API 要点
- **backend.prts.wiki 不可达**（DNS 解析失败），WebFetch 也被企业策略拦截
- 可用：`https://prts.wiki/api.php`（MediaWiki API，本机 curl.exe 直连）
- 全量干员列表：`action=query&list=categorymembers&cmtitle=分类:干员&cmlimit=500`（cmtitle 必须 URL 编码）
- 批量 wikitext：`action=query&titles=A|B|C&prop=revisions&rvprop=content&rvslots=main&format=json&formatversion=2`（每批 ~40 个标题，`|` 编码为 %7C）
- 提取字段：`|上线时间=YYYY-MM-DD`（在 `{{干员获得方式}}` 模板内）、`|干员id=char_NNN_xxx`（NNN = 鹰角实装顺序号，异格是 1001+ 段）
- 开服干员同日实装，靠 char 编号排次级顺序

## PowerShell 5.1 坑
- **无 BOM 的 .ps1 文件会被按 ANSI 解析，脚本内中文字面量变乱码** → 正则里的中文用码点构造：`[char]0x4E0A + [char]0x7EBF...`，脚本文件保持纯 ASCII
- 交互式命令行的中文正常
- 数据文件（UTF-8）读写显式加 `-Encoding UTF8`
- 生成 JSON 用 StringBuilder 手工拼接，避免 ConvertTo-Json 把中文转成 \uXXXX

## 后续任务
- Icons 新增资产后，重新跑匹配+排序+生成即可（PRTS 数据有 456 个干员页面，Icons 目前覆盖 136 个）
- 参见 [[operators-json-workflow]]
