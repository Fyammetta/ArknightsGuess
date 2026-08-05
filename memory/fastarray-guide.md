---
name: fastarray-guide
description: UE FastArray 声明、定义、使用完整指南，含 Kili 项目 WorldLog FastArray 实例分析
metadata:
  type: reference
---

# UE FastArray 完整指南

## 一、概述

`FFastArraySerializer` 是 UE 网络层提供的增量复制容器。与普通 `TArray` 的 Replicated 相比：

| | Replicated TArray | FastArray |
|---|---|---|
| 增量同步 | 全量重传每个元素 | ✅ 只传变更项 |
| 客户端回调 | 无，需 OnRep 手动 diff | ✅ Pre/Post Replicated 回调 |
| 带宽 | O(n) 每次 | O(Δ) 仅变更 |
| 适用场景 | 小数组（<10 项） | 频繁增删改的列表 |

---

## 二、声明（Declaration）

三步缺一不可，全放在同一个头文件里：

### Step 1: Item 结构体 — 继承 `FFastArraySerializerItem`

```cpp
USTRUCT()
struct FMyItem : public FFastArraySerializerItem
{
    GENERATED_BODY()

    UPROPERTY()
    int32 Value;          // 要复制的数据，必须 UPROPERTY
};
```

### Step 2: Array 结构体 — 继承 `FFastArraySerializer`

```cpp
USTRUCT()
struct FMyItemArray : public FFastArraySerializer
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FMyItem> Items;                    // 必须叫 Items 或由模板指定

    // 必须实现 NetDeltaSerialize
    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
    {
        return FFastArraySerializer::FastArrayDeltaSerialize<FMyItem, FMyItemArray>(
            Items, DeltaParams, *this);
    }
};
```

### Step 3: 注册类型特征 — `TStructOpsTypeTraits`

```cpp
template<>
struct TStructOpsTypeTraits<FMyItemArray> : public TStructOpsTypeTraitsBase2<FMyItemArray>
{
    enum { WithNetDeltaSerializer = true };
};
```

> **关键**：`TStructOpsTypeTraits` 必须写在 `.h` 里、必须在 `USTRUCT` 的 `GENERATED_BODY` 可见范围内。UHT 通过这个 trait 得知该结构体使用定制的序列化器。缺少它，`NetDeltaSerialize` 不会被调用，FastArray 退化为普通 UPROPERTY 复制（全量）。

---

## 三、定义（Definition）— 可选的 Callback

FastArray 支持三个回调，定义在 **Item 结构体的 `.cpp`** 里：

```cpp
// 元素即将从客户端移除
void FMyItem::PreReplicatedRemove(const FMyItemArray& InArraySerializer)
{
    // 客户端 UI 清理：隐藏对应的 widget
}

// 元素新添加到客户端
void FMyItem::PostReplicatedAdd(const FMyItemArray& InArraySerializer)
{
    // 客户端 UI 创建：显示新的 widget
    // 可以通过 InArraySerializer 拿到 Owner 引用
}

// 元素在客户端被更新
void FMyItem::PostReplicatedChange(const FMyItemArray& InArraySerializer)
{
    // 客户端 UI 刷新：更新已有 widget 的显示
}
```

> **注意**：这三个回调的签名是 UE 反射系统写死的，参数类型必须是 Array 结构体的 const 引用。如果 Array 不需要区分增/删/改，可以不实现这三个回调。

---

## 四、使用（Usage）

### 4.1 在 Actor/Component 上声明属性

```cpp
UCLASS()
class AMyActor : public AActor
{
    UPROPERTY(Replicated)
    FMyItemArray MyItems;
};

// 必须加 DOREPLIFETIME（和其他 Replicated 属性一样）
void AMyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMyActor, MyItems);
}
```

### 4.2 服务端：增 / 删 / 改 / 清空

```cpp
// 添加
FMyItem& NewItem = MyItems.Items.AddDefaulted_GetRef();
NewItem.Value = 42;
MyItems.MarkItemDirty(NewItem);    // 标记该项为脏，触发增量同步

// 修改
MyItems.Items[0].Value = 100;
MyItems.MarkItemDirty(MyItems.Items[0]);

// 删除
MyItems.Items.RemoveAt(0);
MyItems.MarkArrayDirty();          // 元素数量变了，标记整个数组

// 清空
MyItems.Items.Empty();
MyItems.MarkArrayDirty();
```

### 4.3 辅助方法（推荐封装在 Array 结构体里）

```cpp
void FMyItemArray::AddItem(int32 Value)
{
    FMyItem& Item = Items.AddDefaulted_GetRef();
    Item.Value = Value;
    MarkItemDirty(Item);
}

void FMyItemArray::RemoveItem(int32 Index)
{
    if (Items.IsValidIndex(Index))
    {
        Items.RemoveAt(Index);
        MarkArrayDirty();
    }
}
```

### 4.4 客户端：感知变更

两种方式：

**方式 A**：实现 Item 的 `PostReplicatedAdd` / `PostReplicatedChange` / `PreReplicatedRemove`（见第三节）。

**方式 B**：Array 结构体持有 Owner 的弱引用，回调中通知 Owner：

```cpp
// Array 结构体里
UPROPERTY(NotReplicated)
TWeakObjectPtr<AMyActor> Owner;

// Item 的 PostReplicatedAdd 回调里
void FMyItem::PostReplicatedAdd(const FMyItemArray& InArray)
{
    if (InArray.Owner.IsValid())
    {
        InArray.Owner->OnItemAdded(*this);
    }
}
```

---

## 五、Build.cs 依赖

使用 FastArray 需要依赖 `NetCore` 模块：

```csharp
PublicDependencyModuleNames.Add("NetCore");
```

对应的头文件：

```cpp
#include "Net/Serialization/FastArraySerializer.h"
```

---

## 六、在 GameState 上直接使用 FastArray（简写版）

本项目 `ArknightsGuess` 的 `FTriedAnswerArray` 即是此模式：

```cpp
// ---- OperatorTypes.h ----

USTRUCT()
struct FTriedAnswerEntry : public FFastArraySerializerItem
{
    GENERATED_BODY()
    UPROPERTY()
    FName OperatorName;
};

USTRUCT()
struct FTriedAnswerArray : public FFastArraySerializer
{
    GENERATED_BODY()
    UPROPERTY()
    TArray<FTriedAnswerEntry> Items;

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
    {
        return FFastArraySerializer::FastArrayDeltaSerialize<FTriedAnswerEntry, FTriedAnswerArray>(
            Items, DeltaParams, *this);
    }
};

template<>
struct TStructOpsTypeTraits<FTriedAnswerArray> : public TStructOpsTypeTraitsBase2<FTriedAnswerArray>
{
    enum { WithNetDeltaSerializer = true };
};
```

```cpp
// ---- GameState ----
UPROPERTY(Replicated)
FTriedAnswerArray TriedAnswers;

// GetLifetimeReplicatedProps 里
DOREPLIFETIME(AGuessGameStateBase, TriedAnswers);

// 服务端添加
FTriedAnswerEntry& Entry = TriedAnswers.Items.AddDefaulted_GetRef();
Entry.OperatorName = GuessName;
TriedAnswers.MarkItemDirty(Entry);
```

---

## 七、Kili 项目 WorldLog FastArray 实例分析

### 7.1 背景

Kili 项目中有一个 WorldLog 系统——NPC 的行为日志（谁在什么时候做了什么），需要从 DS 复制到所有客户端显示。日志数量大（数百条），且随时间持续增长。

### 7.2 旧版实现（FastArray，已废弃）

代码位于 `NexusInfo.h:285-352`：

```cpp
// Item: 包装一条完整的 WorldLog
USTRUCT()
struct FAWWorldLogEntry : public FFastArraySerializerItem
{
    UPROPERTY()
    FAWWorldLog LogData;           // 完整日志（含多个 FString、TArray 字段）

    void PreReplicatedRemove(const FAWWorldLogArray& InArraySerializer);
    void PostReplicatedAdd(const FAWWorldLogArray& InArraySerializer);
    void PostReplicatedChange(const FAWWorldLogArray& InArraySerializer);
};

// Array: 持有 GameState 弱引用，封装 Add / GetAll / Clear
USTRUCT()
struct FAWWorldLogArray : public FFastArraySerializer
{
    UPROPERTY()
    TArray<FAWWorldLogEntry> Items;

    UPROPERTY(NotReplicated)
    TWeakObjectPtr<AKiliGameStateBase> OwnerGameState;   // 回调桥梁

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams) { /* ... */ }

    void AddLog(const FAWWorldLog& NewLog) { Items.Add(...); MarkItemDirty(Items.Last()); }
    TArray<FAWWorldLog> GetAllLogs() const { /* flatten */ }
    void ClearLogs() { Items.Empty(); MarkArrayDirty(); }
};

template<>
struct TStructOpsTypeTraits<FAWWorldLogArray> : public TStructOpsTypeTraitsBase2<FAWWorldLogArray>
{
    enum { WithNetDeltaSerializer = true };
};
```

挂载在 GameState 上（`KiliGameStateBase.h:128`）：

```cpp
/** @deprecated */
UPROPERTY(Replicated)
FAWWorldLogArray ReplicatedWorldLogs;
```

客户端回调链路：

```
[Server] AddLog()
    → MarkItemDirty()
    → UEReplicationGraph 检测到变更
    → 调用 NetDeltaSerialize → 只序列化增量条目
    → 发送到客户端

[Client] 收到增量数据
    → FFastArraySerializer 内部 reconcile Items 列表
    → 对新条目调用 FAWWorldLogEntry::PostReplicatedAdd()
    → 通过 OwnerGameState 弱引用 → OnWorldLogReplicated()
    → 转发到 UWorldLogSubsystem → UI 更新
```

### 7.3 为什么被废弃

```cpp
// NexusInfo.h:277-279
// ========== World Log Replication (FFastArraySerializer) [DEPRECATED] ==========
// Deprecated: Use UWorldLogReplicationComponent and UWorldLogReplicationInterface instead.
```

**核心问题：**

1. **不可控的时序**：FastArray 依赖 GameState 的属性复制通道。WorldLog 在游戏过程中持续增长（数百条），而 GameState 的 `ReplicatedWorldLogs` 在 InitialDirty 阶段是全量复制——新客户端加入时，数百条历史的 WorldLog 全部走属性复制，而非 RPC。

2. **无传输控制**：FastArray 的增量同步由 UE 的 ReplicationGraph 调度，开发者无法控制"一次发多少条"、"客户端确认了再发下一批"。WorldLog 的量级决定了它需要**分块传输 + ACK 确认**机制。

3. **FAWWorldLog 体积大**：每条日志包含多个 `FString` 字段（FormattedLog、Time、LocationName、ActionName 等）、`TArray<FName>`、`TArray<FString>`。即便增量发送，单条日志的序列化开销也不小。

4. **无历史数据按需加载**：FastArray 只有"当前全量"语义，不支持"客户端请求某个时间范围的日志"。新版 `UWorldLogReplicationComponent` 通过 DB 归档 + 客户端请求实现了历史日志的分段拉取。

### 7.4 新版替代方案

```
[DS]                                [Client]
UWorldLogReplicationComponent       UWorldLogReplicationInterface (PlayerController)

1. Subsystem::AddLog()
2. → Component::BeginChunkTransfer()
3. → ServerSentLogChunk(chunk, idx)  ──Multicast──→  ClientReceive → Subsystem::OnLogChunkReceived
4.   (等待所有客户端 ACK)
5. → 客户端 ServerRequestNextChunk(idx)  ──ServerRPC──→  Component::OnRequestNextChunk()
6. → 发下一块 或 完成
```

**优势对比：**

| 特性 | FastArray（旧） | Chunked RPC（新） |
|------|----------------|-------------------|
| 传输方式 | 属性复制通道，不可控 | Multicast RPC，显式控制 |
| 分块 | 不支持 | 支持，每块 N 条 |
| 客户端确认 | 无（依赖可靠复制） | ACK + 看门狗重发 |
| 新客户端加入 | 全量 InitialDirty | 可通过 RPC 拉取历史 |
| 历史数据 | 只存当前 | DB 归档 + 按需请求 |
| 传输进度可见 | 无 | ChunkCount / TotalCount |

### 7.5 启示

- **小数据（几十项以内）、频繁变更** → FastArray 是最优解（如 `TriedAnswers`、装备列表、Buff 列表）
- **大数据（数百项）、单向增长** → 分块 RPC 更好（如聊天日志、WorldLog、交易记录）
- **FastArray 是"增量同步"工具，不是"流控"工具**——如果你的数据需要流控（分块、暂停、重传），就应该用 RPC 而非属性复制

---

## 八、与项目内 `FTriedAnswerArray` 的对比

| | Kili FAWWorldLogArray | ArknightsGuess FTriedAnswerArray |
|---|---|---|
| Item 大小 | 大（10+ 字段） | 极小（1 个 FName） |
| 数组规模 | 数百条持续增长 | 每轮 ≤ MaxGuessCount（~10 条） |
| 变更频率 | 高，NPC 持续产生日志 | 低，每猜测一次加一条 |
| Owner 引用 | 有（弱引用 GameState） | 无（不区分增删，客户端只需知道名字） |
| Callback | PostReplicatedAdd → Owner → UI | 不需要（纯数据，UI 通过 OnRep 整体刷新） |
| 是否适合 | ❌ 已废弃换成分块 RPC | ✅ 完美匹配 |

---

## 九、快速检查清单

写 FastArray 时自查：

- [ ] `.h` 里 `#include "Net/Serialization/FastArraySerializer.h"`
- [ ] `Build.cs` 里加了 `"NetCore"`
- [ ] Item 继承 `FFastArraySerializerItem`，字段都是 `UPROPERTY()`
- [ ] Array 继承 `FFastArraySerializer`，有 `TArray<Item> Items`
- [ ] Array 实现了 `bool NetDeltaSerialize(FNetDeltaSerializeInfo&)`
- [ ] `.h` 末尾有 `template<> struct TStructOpsTypeTraits<FArray>` 且 `WithNetDeltaSerializer = true`
- [ ] Owner Actor/Component 里有 `UPROPERTY(Replicated) FArray MyArray`
- [ ] `GetLifetimeReplicatedProps` 里有 `DOREPLIFETIME` 这一项
- [ ] 服务端修改后用 `MarkItemDirty()` / `MarkArrayDirty()`
