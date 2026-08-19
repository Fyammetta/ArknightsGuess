// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LanDiscoverySubsystem.generated.h"

// 自建局域网发现的 UDP 端口(与游戏端口 7777 分开)
#define LAN_DISCOVERY_PORT 7788
// 搜索端广播查询周期(秒)
#define LAN_DISCOVERY_INTERVAL 2.0f
// 房间超过该秒数未刷新则从列表移除
#define LAN_DISCOVERY_TIMEOUT 8.0f

class FUdpSocketReceiver;
class FArrayReader;
struct FIPv4Endpoint;

// 发现的房间信息(普通结构体,不参与反射)
struct FLanRoomInfo
{
	FString RoomName;
	int32 GamePort = 7777;
	// 建房端上报的全部本机 IPv4
	TArray<FString> IPs;
	// 按子网匹配选出的"最可达"地址
	FString BestIP;
	// 回包源地址(一般即热点/网关接口)
	FString SourceIP;
	int32 PingMs = 0;
	double LastSeenTime = 0.0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLanRoomsUpdated);

/**
 * 方案 3:热点模式局域网直连发现。
 *
 * 背景:UE 的 FindSessions(bIsLanQuery) 在安卓热点下会把 LAN beacon socket 绑定到
 * GetLocalHostAddr() 返回的"移动数据接口"(如 10.29.205.185),而 PC 广播到达热点接口
 * (如 10.230.207.71)时没有监听,导致搜不到房间。引擎 installed 版无源码不可改,
 * 因此在应用层自建一套 UDP 发现:
 *   - 建房端:UDP socket 绑 0.0.0.0:7788,收到查询包后回包,payload 携带全部 IPv4 + 端口 + 房间名
 *   - 搜索端:UDP socket 绑 0.0.0.0:7788,定时广播查询包,收包解析后按子网匹配选出可达 IP
 * 关键点:绑 0.0.0.0 才能同时收到"热点接口"上的广播;回包源自动是热点接口,PC 必达。
 */
UCLASS()
class ARKNIGHTSGUESS_API ULanDiscoverySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 枚举本机全部 IPv4(跨平台;安卓额外用 JNI 枚举,含热点接口)。 */
	static TArray<FString> GetAllLocalIPv4();

	// ---- 建房端 ----
	UFUNCTION(BlueprintCallable)
	bool StartAdvertising(const FString& RoomName, int32 GamePort);
	UFUNCTION(BlueprintCallable)
	void StopAdvertising();
	bool IsAdvertising() const { return AdvertiseSocket != nullptr; }

	// ---- 搜索端 ----
	UFUNCTION(BlueprintCallable)
	bool StartSearching();
	UFUNCTION(BlueprintCallable)
	void StopSearching();
	bool IsSearching() const { return SearchSocket != nullptr; }

	const TArray<FLanRoomInfo>& GetFoundRooms() const { return FoundRooms; }

	/** 房间列表刷新(游戏线程触发),UI 监听后重绘。 */
	UPROPERTY(BlueprintAssignable)
	FOnLanRoomsUpdated OnRoomsUpdated;

private:
	// ---- 协议 ----
	static constexpr uint8 MagicQuery[4]   = { 'A', 'G', 'Q', 'R' };
	static constexpr uint8 MagicRespond[4] = { 'A', 'G', 'R', 'M' };
	static constexpr uint16 ProtoVer = 1;

	static void AppendU16(TArray<uint8>& Out, uint16 V);
	static void AppendStr(TArray<uint8>& Out, const FString& S);
	static bool ReadU16(const TArray<uint8>& Data, int32& Offset, uint16& Out);
	static bool ReadStr(const TArray<uint8>& Data, int32& Offset, FString& Out);

	// 收包处理(游戏线程执行)
	void HandlePacket(const TArray<uint8>& Raw, const FIPv4Endpoint& Endpoint);
	void HandleQuery(const TSharedRef<FInternetAddr>& From);           // 建房端收到查询 -> 回包
	void HandleResponse(const TArray<uint8>& Payload, const FString& FromIp, int32 FromPort); // 搜索端收到回应

	void BuildResponse(TArray<uint8>& Out) const;

	// 搜索端定时广播查询 + 清理过期房间
	void TickSearch();

	// 子网匹配:从 IPs 里挑出与 LocalIPs 同网段(前三段相同)的地址
	static FString PickBestIP(const TArray<FString>& IPs, const TArray<FString>& LocalIPs);
	static bool SameSubnet(const FString& A, const FString& B);

	void CloseSockets(){};
	void StopSearchTimer();

#if PLATFORM_ANDROID
	static TArray<FString> AndroidEnumerateIPv4();
#endif

	FSocket* AdvertiseSocket = nullptr;
	TSharedPtr<FUdpSocketReceiver> AdvertiseReceiver;

	FSocket* SearchSocket = nullptr;
	TSharedPtr<FUdpSocketReceiver> SearchReceiver;

	FTimerHandle SearchTimerHandle;

	// 广告数据快照:接收线程通过 TSharedPtr 读取,避免与游戏线程并发读写
	TSharedPtr<FString> AdvertisedRoomName;
	TSharedPtr<int32> AdvertisedPort;
	TArray<FString> LocalIPv4Cache;

	TArray<FLanRoomInfo> FoundRooms;
	bool bSearchRequestPending = false;
	double LastQuerySentTime = 0.0;
};
