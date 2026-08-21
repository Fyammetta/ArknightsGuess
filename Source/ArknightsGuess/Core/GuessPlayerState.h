// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "UI/PlayerIconInterface.h"
#include "GuessPlayerState.generated.h"

/**
 * 
 */

DECLARE_MULTICAST_DELEGATE(FPlayerIconChangeDelegate);
UCLASS()
class ARKNIGHTSGUESS_API AGuessPlayerState : public APlayerState, public IPlayerIconInterface
{
	GENERATED_BODY()
	
private:
	bool bHasInitializedByRep = false;
	
	UPROPERTY(ReplicatedUsing="OnRep_PlayerIcon")
	UTexture2D* PlayerIcon;
	
public:	
	FPlayerIconChangeDelegate OnPlayerIconChangedDelegate;
	
	UFUNCTION()
	void OnRep_PlayerIcon();
	
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual UTexture2D* GetPlayerIcon() const override;
	
	virtual void ChangePlayerIcon(UTexture2D* Icon) override;
	
	virtual void SeamlessTravelTo(class APlayerState* NewPlayerState) override;
	
	UFUNCTION(Server, Reliable)
	void InitPlayerState(const FSoftObjectPath& Icon, const FString& Name);
	
	void OnLocalPlayerJoined();
	
	virtual TMulticastDelegate<void()>* OnPlayerIconChanged() override { return &OnPlayerIconChangedDelegate;};
	
	virtual bool ShouldShowIcon() override;
};
