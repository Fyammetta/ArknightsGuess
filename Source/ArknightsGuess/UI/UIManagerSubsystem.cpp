// Fill out your copyright notice in the Description page of Project Settings.

#include "UIManagerSubsystem.h"
#include "UIManagerSettings.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "ArknightsGuess.h"

void UUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UUIManagerSubsystem::Deinitialize()
{
	DestroyAllUI();
	Super::Deinitialize();
}

UUIManagerSubsystem* UUIManagerSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World) return nullptr;

	const UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UUIManagerSubsystem>() : nullptr;
}

UUserWidget* UUIManagerSubsystem::ShowUI(FGameplayTag Tag)
{
	UE_LOG(LogArknights, Log, TEXT("[UIMgr] ShowUI | Tag=%s"), *Tag.ToString());
	if (!Tag.IsValid()) { UE_LOG(LogArknights, Warning, TEXT("[UIMgr] ShowUI failed: invalid Tag")); return nullptr; }
	
	// ---- Already active? bring to front ----
	if (TObjectPtr<UUserWidget>* Existing = ActiveUIs.Find(Tag))
	{
		if (*Existing && (*Existing)->IsInViewport())
		{
			// Already visible — nothing to do
			return *Existing;
		}

		// Cached but removed from viewport — re-add
		(*Existing)->AddToViewport(UUIManagerSettings::Get()->DefaultZOrder);
		OnUIShown.Broadcast(Tag);
		return *Existing;
	}

	// ---- Lazy-create ----
	const UUIManagerSettings* Settings = UUIManagerSettings::Get();
	const TSubclassOf<UUserWidget>* ClassPtr = Settings->UIRegistry.Find(Tag);
	if (!ClassPtr || !ClassPtr->Get())
	{
		UE_LOG(LogTemp, Warning, TEXT("[UIManagerSubsystem] No widget class registered for tag %s"), *Tag.ToString());
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		// Fallback: try to get world from game instance
		if (UGameInstance* GI = GetGameInstance())
		{
			World = GI->GetWorld();
		}
	}
	if (!World) { UE_LOG(LogArknights, Warning, TEXT("[UIMgr] ShowUI failed: no World")); return nullptr; }

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) { UE_LOG(LogArknights, Warning, TEXT("[UIMgr] ShowUI failed: no PlayerController")); return nullptr; }

	UUserWidget* Widget = CreateWidget<UUserWidget>(PC, *ClassPtr, Tag.GetTagName());
	if (!Widget) return nullptr;

	Widget->AddToViewport(Settings->DefaultZOrder);
	ActiveUIs.Add(Tag, Widget);
	OnUIShown.Broadcast(Tag);

	return Widget;
}

void UUIManagerSubsystem::HideUI(FGameplayTag Tag)
{
	UE_LOG(LogArknights, Log, TEXT("[UIMgr] HideUI | Tag=%s"), *Tag.ToString());
	if (TObjectPtr<UUserWidget>* Found = ActiveUIs.Find(Tag))
	{
		if (*Found)
		{
			(*Found)->RemoveFromParent();
			OnUIHidden.Broadcast(Tag);
		}
	}
}

void UUIManagerSubsystem::DestroyUI(FGameplayTag Tag)
{
	if (TObjectPtr<UUserWidget>* Found = ActiveUIs.Find(Tag))
	{
		if (*Found)
		{
			(*Found)->RemoveFromParent();
			OnUIHidden.Broadcast(Tag);
		}
		ActiveUIs.Remove(Tag);
	}
}

UUserWidget* UUIManagerSubsystem::GetUI(FGameplayTag Tag) const
{
	if (const TObjectPtr<UUserWidget>* Found = ActiveUIs.Find(Tag))
	{
		return *Found;
	}
	return nullptr;
}

bool UUIManagerSubsystem::IsUIShown(FGameplayTag Tag) const
{
	if (const TObjectPtr<UUserWidget>* Found = ActiveUIs.Find(Tag))
	{
		return (*Found) && (*Found)->IsInViewport();
	}
	return false;
}

void UUIManagerSubsystem::HideAllUI()
{
	for (auto& [Tag, Widget] : ActiveUIs)
	{
		if (Widget)
		{
			Widget->RemoveFromParent();
			OnUIHidden.Broadcast(Tag);
		}
	}
}

void UUIManagerSubsystem::DestroyAllUI()
{
	for (auto& [Tag, Widget] : ActiveUIs)
	{
		if (Widget)
		{
			Widget->RemoveFromParent();
			OnUIHidden.Broadcast(Tag);
		}
	}
	ActiveUIs.Empty();
}
