# 多人游戏卡在 Loading UI —— 诊断报告

## 一、结论（TL;DR）

**客户端在无缝旅行到 `Map_GuessGame` 后卡在 Loading 的根因，是 `AGuessPlayerState::InitPlayerState` 的 Server RPC 无法送达服务器（`No owning connection`），导致客户端永远收不到图标复制，`OnLocalPlayerJoined` 永不触发，Loading 永不隐藏。**

这只是表面故障。日志还暴露了更深层的两个架构问题：**客户端 `OperatorSubsystem` 的 Mode 从未被同步**（`NetMulticast_StartGame` 只定义、从未被调用）。

---

## 二、日志时间线（客户端视角，`NetMode=3`）

| 行号 | 日志 | 含义 |
|------|------|------|
| 840 | `[PC] JoinServer | Url=127.0.0.1:7777` | 客户端加入主机 |
| 919 | Welcome，Level=`Map_MultiplayRoom` | 进入房间 |
| 946 | `[AGuessPlayerState::BeginPlay] NetMode=3` | 房间内 PlayerState |
| 947 | Warning: `No owning connection ... InitPlayerState` | **房间内已出现此警告** |
| 952 | Loading 隐藏 | 房间 UI 正常出现 |
| 957 | `Set up player list for 2 players` | 两个玩家都在房间里 ✅ |
| 961 | `SeamlessTravel to: /Game/Maps/Map_GuessGame` | 房主开局 |
| 1011 | **`GenerateGameplayComponent failed: no GameMode selected`** | **客户端 Mode 为空** |
| 1016 | `[DefaultPC] BeginPlay` | 客户端 PC 初始化 |
| 1018 | `ShowUI | Tag=Game.GuessGame` | 尝试显示游戏 UI |
| 1019 | **`PIE: Error: CreateWidget cannot be used on Player Controller with no attached player`** | **PC 无 Player 附着，UI 创建失败** |
| 1022 | `[AGuessPlayerState::BeginPlay] NetMode=3` | 游戏地图 PlayerState |
| 1023 | `ShowUI | Tag=Main.Loading` | **Loading 再次显示** |
| 1024 | `[AGuessPlayerState::InitPlayerState] Send` | 发送 Server RPC |
| 1025-1028 | 又一轮 AddPlayerState / BeginPlay / ShowUI Loading | 出现第二个 PlayerState |
| 1029 | **`No owning connection for actor GuessPlayerState_2147482247. Function InitPlayerState will not be processed.`** | **RPC 发送失败，卡死的直接原因** |
| 1030 | `InitPlayerState Send` | 又一次失败的重试 |

**Loading 在此后从未被隐藏**，玩家永久卡在 Loading UI。

---

## 三、为什么房主单人能玩，客户端却卡死

### 房主（ListenServer，单人）
- `GuessPlayerState::BeginPlay` 中 `HasAuthority()==true`，走的是本地分支：
  - 不显示 Loading（第 23 行守卫）
  - 不发送 `InitPlayerState` RPC
- 房主本地 `PlayerIcon` 直接设置成功，`OnLocalPlayerJoined` 正常触发 → 隐藏 Loading、显示游戏 UI。
- 因此**房主单人流程完全绕开了加载/复制链路**，一切正常。

### 客户端（远程加入）
- `HasAuthority()==false`，走远程分支：
  - 显示 Loading（第 27 行）
  - 发送 `InitPlayerState` Server RPC（第 35 行）
- 但该 RPC 因 **`No owning connection`** 失败 → 服务器不执行 `ChangePlayerIcon` → `PlayerIcon` 不复制回客户端 → `OnRep_PlayerIcon` 不触发 → `OnLocalPlayerJoined` 不执行 → **Loading 永不隐藏**。

---

## 四、根因分析

### 根因 A（直接卡死点）：`GuessPlayerState` 的 Server RPC 无 owning connection

```
[AGuessPlayerState::BeginPlay] NetMode = 3     ← 客户端显示 Loading
[AGuessPlayerState::InitPlayerState] Send      ← 发送 RPC
UNetDriver::ProcessRemoteFunction: No owning connection for actor GuessPlayerState... Function InitPlayerState will not be processed.
```

`GuessPlayerState.cpp` 第 31-37 行：

```cpp
if (!HasAuthority() || PlayerIcon == nullptr)
{
    FString Name = UGuessGamerSettings::GetPlayerName();
    UTexture2D* Icon = UGuessGamerSettings::GetPlayerIcon();
    InitPlayerState(Icon->GetPathName(), Name);   // Server RPC，在 BeginPlay 里直接发
    ...
}
```

客户端在无缝旅行后、PlayerState 的复制通道/owning connection 尚未就绪时就发出 RPC，RPC 被丢弃。这属于 **时序问题**：在 `BeginPlay` 里直接发 Server RPC 不可靠，尤其是无缝旅行刚完成时。

日志 1025-1029 显示客户端在游戏地图出现了**两个** PlayerState（一个正常，一个 `_2147482247` 无 owning connection），正是旅行期间 PC/PlayerState 重建时序混乱的表现。

### 根因 B（架构缺失）：客户端从未拿到游戏 Mode

```
[DefaultGS] GenerateGameplayComponent failed: no GameMode selected   ← 客户端
```

`DefaultGameStateBase::GenerateGameplayComponent`（第 123-127 行）依赖 `Subsystem->GetGameplayMode()`，但客户端该值恒空。

原因是 **`DefaultGameStateBase::NetMulticast_StartGame` 只定义、从未被调用**：

- 服务器 `StartGame_Implementation`（`DefaultPlayerController.cpp` 第 262 行）只调用 `Subsystem->StartUp(Mode)`（本地生效）+ `ServerTravel`。
- 没有任何地方调用 `GS->NetMulticast_StartGame(Mode)` 把 Mode 广播给客户端。
- 因此客户端 `OperatorSubsystem::GuessMode` 永远为空，游戏玩法组件（GameModeComponent）无法生成。

### 根因 C（关联隐患）：PC 类注释与实现不符

- 注释：`DefaultPlayerController.h` 第 17 行 "AGuesserPlayerController inherits from this"
- 实际：`GuesserPlayerController.h` 第 14 行 `class AGuesserPlayerController : public APlayerController`（**直接继承 APlayerController，未继承 ADefaultPlayerController**）

日志 1016/1019 显示旅行期间 PC 是 `BP_DefaultPlayerController_C`，而退出时（1045 行）连接 PC 是 `BP_GuessPlayerController_C`——即 **PC 类在旅行中途切换**，切换过程中的关联混乱是 `no Player attached`（1019 行）和 RPC 无连接（1029 行）的直接诱因之一。

---

## 五、修复方案（仅建议，未改代码）

> 遵循你的约定：默认只提供方案，不改动代码文件。

### 方案 1（针对根因 A）：让 Loading 的隐藏不依赖脆弱的 RPC 时序
- 不要依赖 `InitPlayerState` Server RPC 成功后才隐藏 Loading。
- 建议：客户端在 `AGuessPlayerState::BeginPlay` 里，把 Loading 隐藏逻辑放到**等 PlayerIcon 复制到**之后，并加一个**超时兜底**（例如 2-3 秒后无论如何隐藏 Loading）。
- 或：把 `InitPlayerState` 的发送时机从 `BeginPlay` 延迟到 `PostLogin`/`OnRep` 之后、或 PC 附着完成之后。

### 方案 2（针对根因 B）：在开局时把 Mode 广播给所有客户端
- 在服务器 `StartGame_Implementation` 中，`ServerTravel` 之前调用：
  ```cpp
  if (auto* GS = GetWorld()->GetGameState<ADefaultGameStateBase>())
      GS->NetMulticast_StartGame(Mode);
  ```
- 并让客户端在无缝旅行后、新的 GameState 就绪时，确保 `OperatorSubsystem` 已持有 Mode。

### 方案 3（针对根因 C）：让 `AGuesserPlayerController` 真正继承 `ADefaultPlayerController`
- 修正继承关系，使游戏内 PC 具备 `ADefaultPlayerController` 的全部 UI/RPC 能力，避免旅行中途 PC 类切换导致的关联断裂。

---

## 六、优先级

1. **根因 A**（RPC 时序 + Loading 兜底）：不修，客户端必然卡 Loading。**最高优先级**
2. **根因 B**（Mode 未同步）：不修，客户端即使进入游戏，玩法组件也无法生成。**高优先级**
3. **根因 C**（PC 继承不符）：影响 PC 能力完整性与类切换，属于深水区隐患。**建议尽快核对**
