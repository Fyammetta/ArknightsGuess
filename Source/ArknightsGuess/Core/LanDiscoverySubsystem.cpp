// Fill out your copyright notice in the Description page of Project Settings.


#include "LanDiscoverySubsystem.h"

#include "ArknightsGuess.h"
#include "Async/Async.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "IPAddress.h"
#include "SocketSubsystem.h"
#include "TimerManager.h"
#include "Common/UdpSocketReceiver.h"

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h" // FAndroidApplication::GetJavaEnv (UE5.4: 位于 ApplicationCore)
#endif

// ============================================================
//  生命周期
// ============================================================

void ULanDiscoverySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LocalIPv4Cache = GetAllLocalIPv4();
	UE_LOG(LogArknights, Log, TEXT("[LanDiscovery] initialized, local IPv4: %s"),
		*FString::Join(LocalIPv4Cache, TEXT(", ")));
}

void ULanDiscoverySubsystem::Deinitialize()
{
	StopSearching();
	StopAdvertising();

	Super::Deinitialize();
}

// ============================================================
//  建房端:广播应答
// ============================================================

bool ULanDiscoverySubsystem::StartAdvertising(const FString& RoomName, int32 Port)
{
	if (AdvertiseSocket)
	{
		return true;
	}

	ISocketSubsystem* SockSub = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SockSub)
	{
		return false;
	}

	FSocket* Sock = SockSub->CreateSocket(NAME_DGram, TEXT("LanAdvertise"), false);
	if (!Sock)
	{
		return false;
	}
	Sock->SetBroadcast(true);
	Sock->SetReuseAddr(true);

	bool bValid = false;
	TSharedRef<FInternetAddr> BindAddr = SockSub->CreateInternetAddr();
	BindAddr->SetIp(TEXT("0.0.0.0"), bValid);
	BindAddr->SetPort(LAN_DISCOVERY_PORT);

	if (!bValid || !Sock->Bind(*BindAddr))
	{
		SockSub->DestroySocket(Sock);
		UE_LOG(LogArknights, Warning, TEXT("[LanAdv] bind 0.0.0.0:%d failed (port in use?)"), LAN_DISCOVERY_PORT);
		return false;
	}

	AdvertiseSocket = Sock;
	AdvertisedRoomName = MakeShared<FString>(RoomName);
	AdvertisedPort = MakeShared<int32>(Port > 0 ? Port : 7777);
	LocalIPv4Cache = GetAllLocalIPv4();

	AdvertiseReceiver = MakeShared<FUdpSocketReceiver>(Sock, FTimespan::FromMilliseconds(100), TEXT("LanAdvReceiver"));
	AdvertiseReceiver->OnDataReceived().BindLambda([this](const FArrayReaderPtr& Data, const FIPv4Endpoint& Endpoint)
	{
		// 接收线程:先把数据拷成 TArray<uint8> 再投递,避免跨线程持有 FArrayReaderPtr
		TArray<uint8> Raw;
		if (Data.IsValid() && Data->Num() > 0)
		{
			Raw.SetNumUninitialized(Data->Num());
			FMemory::Memcpy(Raw.GetData(), Data->GetData(), Data->Num());
		}
		FIPv4Endpoint EndpointCopy = Endpoint;
		AsyncTask(ENamedThreads::GameThread, [this, Raw, EndpointCopy]()
		{
			// 游戏线程收尾:确保子系统还没被销毁
			if (GetGameInstance())
			{
				HandlePacket(Raw, EndpointCopy);
			}
		});
	});
	AdvertiseReceiver->Start();

	UE_LOG(LogArknights, Log, TEXT("[LanAdv] advertising room='%s' port=%d on 0.0.0.0:%d, IPs=[%s]"),
		*RoomName, Port, LAN_DISCOVERY_PORT, *FString::Join(LocalIPv4Cache, TEXT(", ")));
	return true;
}

void ULanDiscoverySubsystem::StopAdvertising()
{
	if (AdvertiseReceiver.IsValid())
	{
		AdvertiseReceiver->Stop();
		AdvertiseReceiver.Reset();
	}

	if (AdvertiseSocket)
	{
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(AdvertiseSocket);
		AdvertiseSocket = nullptr;
	}

	AdvertisedRoomName.Reset();
	AdvertisedPort.Reset();
}

// ============================================================
//  搜索端:定时广播查询
// ============================================================

bool ULanDiscoverySubsystem::StartSearching()
{
	if (SearchSocket)
	{
		return true;
	}

	ISocketSubsystem* SockSub = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SockSub)
	{
		return false;
	}

	FSocket* Sock = SockSub->CreateSocket(NAME_DGram, TEXT("LanSearch"), false);
	if (!Sock)
	{
		return false;
	}
	Sock->SetBroadcast(true);
	Sock->SetReuseAddr(true);

	bool bValid = false;
	TSharedRef<FInternetAddr> BindAddr = SockSub->CreateInternetAddr();
	BindAddr->SetIp(TEXT("0.0.0.0"), bValid);
	BindAddr->SetPort(LAN_DISCOVERY_PORT);

	if (!bValid || !Sock->Bind(*BindAddr))
	{
		SockSub->DestroySocket(Sock);
		UE_LOG(LogArknights, Warning, TEXT("[LanSearch] bind 0.0.0.0:%d failed (port in use?)"), LAN_DISCOVERY_PORT);
		return false;
	}

	SearchSocket = Sock;
	LocalIPv4Cache = GetAllLocalIPv4();

	SearchReceiver = MakeShared<FUdpSocketReceiver>(Sock, FTimespan::FromMilliseconds(100), TEXT("LanSearchReceiver"));
	SearchReceiver->OnDataReceived().BindLambda([this](const FArrayReaderPtr& Data, const FIPv4Endpoint& Endpoint)
	{
		TArray<uint8> Raw;
		if (Data.IsValid() && Data->Num() > 0)
		{
			Raw.SetNumUninitialized(Data->Num());
			FMemory::Memcpy(Raw.GetData(), Data->GetData(), Data->Num());
		}
		FIPv4Endpoint EndpointCopy = Endpoint;
		AsyncTask(ENamedThreads::GameThread, [this, Raw, EndpointCopy]()
		{
			if (GetGameInstance())
			{
				HandlePacket(Raw, EndpointCopy);
			}
		});
	});
	SearchReceiver->Start();

	// 立即广播一次,并进入周期广播
	TickSearch();
	GetGameInstance()->GetTimerManager().SetTimer(SearchTimerHandle, this, &ULanDiscoverySubsystem::TickSearch, LAN_DISCOVERY_INTERVAL, true);

	UE_LOG(LogArknights, Log, TEXT("[LanSearch] started on 0.0.0.0:%d"), LAN_DISCOVERY_PORT);
	return true;
}

void ULanDiscoverySubsystem::StopSearching()
{
	StopSearchTimer();

	if (SearchReceiver.IsValid())
	{
		SearchReceiver->Stop();
		SearchReceiver.Reset();
	}

	if (SearchSocket)
	{
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SearchSocket);
		SearchSocket = nullptr;
	}

	if (!FoundRooms.IsEmpty())
	{
		FoundRooms.Reset();
		OnRoomsUpdated.Broadcast();
	}
}

void ULanDiscoverySubsystem::StopSearchTimer()
{
	if (UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr)
	{
		World->GetTimerManager().ClearTimer(SearchTimerHandle);
	}
}

void ULanDiscoverySubsystem::TickSearch()
{
	if (!SearchSocket)
	{
		return;
	}

	// 1) 广播查询包:AGQR + ver + (空)客户端名
	TArray<uint8> Query;
	Query.Append(MagicQuery, 4);
	AppendU16(Query, ProtoVer);
	AppendU16(Query, 0);

	TSharedRef<FInternetAddr> BroadcastAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	BroadcastAddr->SetBroadcastAddress();
	BroadcastAddr->SetPort(LAN_DISCOVERY_PORT);

	int32 Sent = 0;
	SearchSocket->SendTo(Query.GetData(), Query.Num(), Sent, *BroadcastAddr);
	LastQuerySentTime = FPlatformTime::Seconds();

	// 2) 清理超时房间
	const double Now = FPlatformTime::Seconds();
	const int32 Before = FoundRooms.Num();
	FoundRooms.RemoveAll([Now](const FLanRoomInfo& R)
	{
		return (Now - R.LastSeenTime) > LAN_DISCOVERY_TIMEOUT;
	});
	if (FoundRooms.Num() != Before)
	{
		OnRoomsUpdated.Broadcast();
	}
}

// ============================================================
//  收包处理(游戏线程)
// ============================================================

void ULanDiscoverySubsystem::HandlePacket(const TArray<uint8>& Raw, const FIPv4Endpoint& Endpoint)
{
	if (Raw.Num() < 6)
	{
		return;
	}

	if (Raw[0] == 'A' && Raw[1] == 'G' && Raw[2] == 'Q' && Raw[3] == 'R')
	{
		HandleQuery(Endpoint.ToInternetAddr());
	}
	else if (Raw[0] == 'A' && Raw[1] == 'G' && Raw[2] == 'R' && Raw[3] == 'M')
	{
		HandleResponse(Raw, Endpoint.Address.ToString(), Endpoint.Port);
	}
}

void ULanDiscoverySubsystem::HandleQuery(const TSharedRef<FInternetAddr>& From)
{
	// 仅建房端且有广告信息时响应
	if (!AdvertiseSocket || !AdvertisedRoomName.IsValid() || !AdvertisedPort.IsValid())
	{
		return;
	}

	TArray<uint8> Response;
	BuildResponse(Response);

	int32 Sent = 0;
	AdvertiseSocket->SendTo(Response.GetData(), Response.Num(), Sent, *From);

	UE_LOG(LogArknights, Log, TEXT("[LanAdv] responded to %s | room='%s' | %d bytes"),
		*From->ToString(true), **AdvertisedRoomName, Response.Num());
}

void ULanDiscoverySubsystem::BuildResponse(TArray<uint8>& Out) const
{
	Out.Reset();
	Out.Append(MagicRespond, 4);
	AppendU16(Out, ProtoVer);
	AppendStr(Out, *AdvertisedRoomName);
	AppendU16(Out, (uint16)*AdvertisedPort);

	TArray<FString> IPs = GetAllLocalIPv4();
	AppendU16(Out, (uint16)FMath::Min(IPs.Num(), 65535));
	for (const FString& Ip : IPs)
	{
		AppendStr(Out, Ip);
	}
}

void ULanDiscoverySubsystem::HandleResponse(const TArray<uint8>& Payload, const FString& FromIp, int32 FromPort)
{
	int32 Offset = 4; // 跳过 Magic

	uint16 Ver = 0;
	if (!ReadU16(Payload, Offset, Ver))
	{
		return;
	}

	FString RoomName;
	if (!ReadStr(Payload, Offset, RoomName))
	{
		return;
	}

	uint16 Port = 0;
	if (!ReadU16(Payload, Offset, Port))
	{
		return;
	}

	uint16 IpCount = 0;
	if (!ReadU16(Payload, Offset, IpCount))
	{
		return;
	}

	TArray<FString> IPs;
	for (uint16 i = 0; i < IpCount; ++i)
	{
		FString Ip;
		if (!ReadStr(Payload, Offset, Ip))
		{
			return;
		}
		if (Ip != TEXT("0.0.0.0") && Ip != TEXT("127.0.0.1") && !Ip.Contains(TEXT(":")))
		{
			IPs.AddUnique(Ip);
		}
	}

	const double Now = FPlatformTime::Seconds();

	FLanRoomInfo* Found = FoundRooms.FindByPredicate([&RoomName](const FLanRoomInfo& R)
	{
		return R.RoomName == RoomName;
	});

	if (Found)
	{
		Found->GamePort = Port > 0 ? Port : 7777;
		Found->IPs = IPs;
		Found->SourceIP = FromIp;
		Found->BestIP = PickBestIP(IPs, LocalIPv4Cache);
		Found->PingMs = LastQuerySentTime > 0
			? FMath::Clamp((int32)((Now - LastQuerySentTime) * 1000.0), 0, 9999)
			: 0;
		Found->LastSeenTime = Now;
	}
	else
	{
		FLanRoomInfo Room;
		Room.RoomName = RoomName;
		Room.GamePort = Port > 0 ? Port : 7777;
		Room.IPs = IPs;
		Room.SourceIP = FromIp;
		Room.BestIP = PickBestIP(IPs, LocalIPv4Cache);
		Room.PingMs = LastQuerySentTime > 0
			? FMath::Clamp((int32)((Now - LastQuerySentTime) * 1000.0), 0, 9999)
			: 0;
		Room.LastSeenTime = Now;
		FoundRooms.Add(Room);

		UE_LOG(LogArknights, Log, TEXT("[LanSearch] found room='%s' port=%d src=%s best=%s"),
			*RoomName, Room.GamePort, *FromIp, *Room.BestIP);
	}

	OnRoomsUpdated.Broadcast();
}

// ============================================================
//  工具:协议编解码
// ============================================================

void ULanDiscoverySubsystem::AppendU16(TArray<uint8>& Out, uint16 V)
{
	Out.Add((uint8)(V & 0xFF));
	Out.Add((uint8)((V >> 8) & 0xFF));
}

void ULanDiscoverySubsystem::AppendStr(TArray<uint8>& Out, const FString& S)
{
	FTCHARToUTF8 Conv(*S);
	const uint16 Len = (uint16)FMath::Min<int32>(Conv.Length(), 65535);
	AppendU16(Out, Len);
	if (Len > 0)
	{
		Out.Append((const uint8*)Conv.Get(), Len);
	}
}

bool ULanDiscoverySubsystem::ReadU16(const TArray<uint8>& Data, int32& Offset, uint16& Out)
{
	if (Offset + 2 > Data.Num())
	{
		return false;
	}
	Out = (uint16)(Data[Offset] | (Data[Offset + 1] << 8));
	Offset += 2;
	return true;
}

bool ULanDiscoverySubsystem::ReadStr(const TArray<uint8>& Data, int32& Offset, FString& Out)
{
	uint16 Len = 0;
	if (!ReadU16(Data, Offset, Len))
	{
		return false;
	}
	if (Offset + (int32)Len > Data.Num())
	{
		return false;
	}
	Out = FString(FUTF8ToTCHAR((const ANSICHAR*)Data.GetData() + Offset, Len));
	Offset += Len;
	return true;
}

// ============================================================
//  工具:IP 挑选与枚举
// ============================================================

FString ULanDiscoverySubsystem::PickBestIP(const TArray<FString>& IPs, const TArray<FString>& LocalIPs)
{
	if (IPs.Num() <= 1)
	{
		return IPs.Num() == 1 ? IPs[0] : FString();
	}

	// 优先:与搜索端同网段(前三段一致)的地址
	for (const FString& Local : LocalIPs)
	{
		for (const FString& Ip : IPs)
		{
			if (SameSubnet(Local, Ip))
			{
				return Ip;
			}
		}
	}

	return IPs[0];
}

bool ULanDiscoverySubsystem::SameSubnet(const FString& A, const FString& B)
{
	auto GetOctets = [](const FString& S) -> TArray<int32>
	{
		TArray<int32> Out;
		TArray<FString> Parts;
		S.ParseIntoArray(Parts, TEXT("."), true);
		for (const FString& P : Parts)
		{
			Out.Add(FCString::Atoi(*P));
		}
		return Out;
	};

	const TArray<int32> PA = GetOctets(A);
	const TArray<int32> PB = GetOctets(B);
	if (PA.Num() < 3 || PB.Num() < 3)
	{
		return false;
	}
	return PA[0] == PB[0] && PA[1] == PB[1] && PA[2] == PB[2];
}

TArray<FString> ULanDiscoverySubsystem::GetAllLocalIPv4()
{
	TArray<FString> Result;
	ISocketSubsystem* SockSub = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SockSub)
	{
		return Result;
	}

	// 1) 默认路由接口(兜底,多网卡安卓下可能是移动数据接口)
	bool bCanBind = false;
	TSharedRef<FInternetAddr> Host = SockSub->GetLocalHostAddr(*GLog, bCanBind);
	if (Host->IsValid())
	{
		const FString S = Host->ToString(false);
		if (!S.IsEmpty() && S != TEXT("0.0.0.0") && S != TEXT("127.0.0.1") && !S.Contains(TEXT(":")))
		{
			Result.AddUnique(S);
		}
	}

	// 2) 可绑定地址列表(通常覆盖所有 UP 接口)
	for (const TSharedRef<FInternetAddr>& Addr : SockSub->GetLocalBindAddresses())
	{
		if (!Addr->IsValid())
		{
			continue;
		}
		const FString S = Addr->ToString(false);
		if (S.IsEmpty() || S == TEXT("0.0.0.0") || S == TEXT("127.0.0.1") || S.Contains(TEXT(":")))
		{
			continue;
		}
		Result.AddUnique(S);
	}

#if PLATFORM_ANDROID
	// 3) 安卓原生枚举(覆盖热点接口等 GetLocalBindAddresses 遗漏的地址)
	for (const FString& S : AndroidEnumerateIPv4())
	{
		if (S.IsEmpty() || S == TEXT("0.0.0.0") || S == TEXT("127.0.0.1") || S.Contains(TEXT(":")))
		{
			continue;
		}
		Result.AddUnique(S);
	}
#endif

	return Result;
}

#if PLATFORM_ANDROID
TArray<FString> ULanDiscoverySubsystem::AndroidEnumerateIPv4()
{
	TArray<FString> Result;

	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (!Env)
	{
		return Result;
	}

	jclass NetIfClass = Env->FindClass("java/net/NetworkInterface");
	jclass InetAddrClass = Env->FindClass("java/net/InetAddress");
	jclass EnumerationClass = Env->FindClass("java/util/Enumeration");
	if (!NetIfClass || !InetAddrClass || !EnumerationClass)
	{
		if (NetIfClass) Env->DeleteLocalRef(NetIfClass);
		if (InetAddrClass) Env->DeleteLocalRef(InetAddrClass);
		if (EnumerationClass) Env->DeleteLocalRef(EnumerationClass);
		return Result;
	}

	jmethodID GetNetworkInterfacesM = Env->GetStaticMethodID(NetIfClass, "getNetworkInterfaces", "()Ljava/util/Enumeration;");
	jmethodID HasMoreM = Env->GetMethodID(EnumerationClass, "hasMoreElements", "()Z");
	jmethodID NextM = Env->GetMethodID(EnumerationClass, "nextElement", "()Ljava/lang/Object;");
	jmethodID GetInetAddrsM = Env->GetMethodID(NetIfClass, "getInetAddresses", "()Ljava/util/Enumeration;");
	jmethodID GetHostAddrM = Env->GetMethodID(InetAddrClass, "getHostAddress", "()Ljava/lang/String;");
	jmethodID IsLoopbackM = Env->GetMethodID(InetAddrClass, "isLoopbackAddress", "()Z");

	jobject NetIfEnum = Env->CallStaticObjectMethod(NetIfClass, GetNetworkInterfacesM);
	if (NetIfEnum)
	{
		while (Env->CallBooleanMethod(NetIfEnum, HasMoreM))
		{
			jobject NetIf = Env->CallObjectMethod(NetIfEnum, NextM);
			if (!NetIf)
			{
				continue;
			}

			jobject AddrEnum = Env->CallObjectMethod(NetIf, GetInetAddrsM);
			if (AddrEnum)
			{
				while (Env->CallBooleanMethod(AddrEnum, HasMoreM))
				{
					jobject Addr = Env->CallObjectMethod(AddrEnum, NextM);
					if (!Addr)
					{
						continue;
					}

					if (!Env->CallBooleanMethod(Addr, IsLoopbackM))
					{
						jstring JIp = (jstring)Env->CallObjectMethod(Addr, GetHostAddrM);
						if (JIp)
						{
							const char* CStr = Env->GetStringUTFChars(JIp, nullptr);
							if (CStr)
							{
								const FString Ip(UTF8_TO_TCHAR(CStr));
								if (Ip.Contains(TEXT(".")))
								{
									Result.AddUnique(Ip);
								}
								Env->ReleaseStringUTFChars(JIp, CStr);
							}
							Env->DeleteLocalRef(JIp);
						}
					}
					Env->DeleteLocalRef(Addr);
				}
				Env->DeleteLocalRef(AddrEnum);
			}
			Env->DeleteLocalRef(NetIf);
		}
		Env->DeleteLocalRef(NetIfEnum);
	}

	Env->DeleteLocalRef(NetIfClass);
	Env->DeleteLocalRef(InetAddrClass);
	Env->DeleteLocalRef(EnumerationClass);

	return Result;
}
#endif
