// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MultiplayRoomUI.generated.h"

class UTextBlock;
class UTileView;
class UWidgetSwitcher;
/**
 * 
 */
UCLASS()
class ARKNIGHTSGUESS_API UMultiplayRoomUI : public UUserWidget
{
	GENERATED_BODY()
	FTimerHandle TimerHandle;
	int32 Size = 4;
	protected:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UWidgetSwitcher* HostCastSwitcher;
	
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UTileView* PlayerList;
	
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UTextBlock* PORT_DisplayText;
	
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UTextBlock* IP_DisplayText;
	
	virtual void NativeConstruct() override;
	
private:
	bool UpdateSize();
	
	float GetDesiredSize() const;

	
	UFUNCTION()
	void OnPlayerJoinedOrLeft(APlayerState* Player = nullptr, bool bWasJoined = true);
	
	
};
