// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "UI/PlayerIconInterface.h"
#include "UObject/SoftObjectPath.h"
#include "GuessPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class ARKNIGHTSGUESS_API AGuessPlayerState : public APlayerState, public IPlayerIconInterface
{
	GENERATED_BODY()
	
private:
	UPROPERTY(ReplicatedUsing=OnRep_PlayerIcon)
	FSoftObjectPath PlayerIcon;
	
public:	
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerIconChanged);

	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual UTexture2D* GetPlayerIcon() const override;
	
	virtual void ChangePlayerIcon(UTexture2D* Icon) override;

	void ChangePlayerIcon(const FSoftObjectPath& IconPath);

	/** 头像复制到达时广播,房间列表据此刷新。 */
	UFUNCTION()
	void OnRep_PlayerIcon();

	UPROPERTY(BlueprintAssignable)
	FOnPlayerIconChanged OnPlayerIconChanged;
	
	virtual void CopyProperties(APlayerState* PlayerState) override;
	
	virtual void SeamlessTravelTo(class APlayerState* NewPlayerState) override;
	

};
