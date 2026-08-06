// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GuesserPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ARKNIGHTSGUESS_API AGuesserPlayerController : public APlayerController
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;
public:
	UFUNCTION(Reliable, NetMulticast, BlueprintCallable)
	void StartGame(const FName& Mode);
	
	UFUNCTION(Reliable, NetMulticast, BlueprintCallable)
	void EndGame();
	
	UFUNCTION(Reliable, Server, BlueprintCallable)
	void ConfirmAnswer(const FName& Answer);

};
