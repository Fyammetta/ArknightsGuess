// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "UI/PlayerIconInterface.h"
#include "GuessPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class ARKNIGHTSGUESS_API AGuessPlayerState : public APlayerState, public IPlayerIconInterface
{
	GENERATED_BODY()
	
private:
	UPROPERTY(Replicated)
	UTexture2D* PlayerIcon;
	
public:	
	
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual UTexture2D* GetPlayerIcon() const override;
	
	virtual void ChangePlayerIcon(UTexture2D* Icon) override;
	
	virtual void CopyProperties(APlayerState* PlayerState) override;
	
	virtual void SeamlessTravelTo(class APlayerState* NewPlayerState) override;
	

};
