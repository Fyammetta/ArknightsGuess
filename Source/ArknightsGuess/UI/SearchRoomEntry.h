// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ArknightsGuess/Core/LanDiscoverySubsystem.h"
#include "SearchRoomEntry.generated.h"

class UImage;
class UTextBlock;
class UButton;

/**
 * A single row in the room-search result list.
 */
UCLASS()
class ARKNIGHTSGUESS_API USearchRoomEntry : public UUserWidget
{
	GENERATED_BODY()

protected:
	// ---- Widgets ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> Image_Ping;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_PlayerNum;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_RoomName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_Select;

	// The discovered room this entry stands for (方案3 自建发现的房间信息)。
	FLanRoomInfo Target;

public:
	/** Init the display from a discovered room. Called by UGameMainUI::OnLanRoomsUpdated. */
	void InitEntry(const FLanRoomInfo& InRoom);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Button_Select click: resolve the session address and travel to it. */
	UFUNCTION()
	void OnSelectClicked();
};
