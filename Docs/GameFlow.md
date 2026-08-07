# 游戏启动 → Loading → GameHUD 流程

## Tag 映射

| Tag | 用途 | 默认 Widget |
|-----|------|-------------|
| `Main.Loading` | 加载过渡界面 | 在 `Project Settings → UI Manager → UIRegistry` 中配置 |
| `GameMode.Mosaic` | 马赛克模式 HUD | 同上，复用模式 Tag 作为 HUD Tag |

## 时序图

```
[Client]                                          [Server]
                                                      │
  PC->StartGame(GameMode.Mosaic)  ── Server RPC ──►  │
                                                      ▼
                                              GM->StartGame(Mode)
                                                ├── NetMulticast_StartGame(Mode)
                                                │     │
                                                │     ▼  (Multicast to ALL clients)
                                                │   [All] Subsystem->StartUp(Mode)
                                                │     │
                                                │     ├── Load DataTable rows
                                                │     ├── IsGameRunning = true
                                                │     └── OnGuessGameStart.Broadcast()
                                                │           │
  ◄───────────────────────────────────────────────────────┘
  │
  ▼
  PC::OnGameStart()
    ├── LoadingStartTime = now
    └── UIManager->ShowUI(Main.Loading)          ← Loading 出现
                                                      │
                                                      │
                                              GM->StartNewRound()
                                                ├── GetRandomOperatorData()
                                                ├── EnterNewRound(Answer)
                                                │     │
                                                │     └── NetMulticast_SetupOperator(Image, Hints)
                                                │           │
  ◄──────────────────── (Multicast) ─────────────────────┘
  │
  ▼
  PC::OnOperatorDataReady(Image, Hints)
    │
    ├── Elapsed = now - LoadingStartTime
    ├── Remaining = MinLoadingTime - Elapsed
    │
    ├── Remaining > 0 ?
    │     YES → SetTimer(FinishLoading, Remaining)
    │     NO  → FinishLoading()
    │
    ▼
  PC::FinishLoading()
    ├── UIManager->HideUI(Main.Loading)          ← Loading 消失
    └── UIManager->ShowUI(GameMode.Mosaic)       ← GameHUD 出现
```

## 配置入口

### 1. Project Settings → Game → UI Manager

| 设置项 | 默认值 | 说明 |
|--------|--------|------|
| UIRegistry | `{Main.Loading → WBP_Loading, GameMode.Mosaic → WBP_GuessHUD}` | Tag→Widget 映射 |
| MinLoadingTime | `0.5` | Loading 最短显示时长（秒），防闪屏 |
| DefaultZOrder | `0` | AddToViewport 的 Z 序 |

### 2. GuesserPlayerController（蓝图可覆盖）

| 属性 | 默认值 | 说明 |
|------|--------|------|
| InitialUITag | — | BeginPlay 时显示的首个 UI（主菜单） |
| LoadingUITag | `Main.Loading` | 开局加载界面 Tag |
| GameHUDTag | `GameMode.Mosaic` | 游戏 HUD Tag |

## 关键类与职责

| 类 | 职责 |
|----|------|
| `AGuesserPlayerController` | 绑定 Subsystem delegate，管理 loading 最低时长计时器，驱动 UI 切换 |
| `UUIManagerSubsystem` | 懒加载/缓存/显示/隐藏 UI Widget |
| `UUIManagerSettings` | Tag→WidgetClass 映射 + MinLoadingTime 配置 |
| `UOperatorSubsystem` | 广播 `OnGuessGameStart` / `OnOperatorDataReceived` |
| `AGuessGameStateBase` | Multicast RPC 触发 Subsystem 事件 |
| `AGuessGameModeBase` | Server 端游戏逻辑，不关心 UI |

## Delegate 链路

```
AGuessGameStateBase::NetMulticast_StartGame
  └─► UOperatorSubsystem::StartUp
        └─► OnGuessGameStart.Broadcast()

AGuessGameStateBase::NetMulticast_SetupOperator
  └─► UOperatorSubsystem::OnOperatorDataReceived.Broadcast()
```

## 蓝图接入

1. 创建 `WBP_Loading`（Loading 界面 Widget）
2. 创建 `WBP_GuessHUD`（游戏主界面 Widget）
3. 在 `Project Settings → UI Manager → UIRegistry` 添加映射：
   - `Main.Loading` → `WBP_Loading`
   - `GameMode.Mosaic` → `WBP_GuessHUD`
4. 创建继承 `GuesserPlayerController` 的蓝图（如 `BP_GuesserPC`），按需覆盖 Tag
5. GameMode 或 Level 中使用该 PC 蓝图
