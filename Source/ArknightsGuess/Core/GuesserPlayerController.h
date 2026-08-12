// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DefaultPlayerController.h"
#include "GuesserPlayerController.generated.h"

/**
 * In-game PlayerController — inherits UI lifecycle and startup RPCs from ADefaultPlayerController.
 * Adds in-game RPCs: ConfirmAnswer / RequestNextRound.
 */
UCLASS()
class ARKNIGHTSGUESS_API AGuesserPlayerController : public APlayerController
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FGameplayTag InitialUITag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FGameplayTag EntryUITag;
public:
	
	virtual void BeginPlay() override;
	
	UFUNCTION(Reliable, Server, BlueprintCallable)
	void ConfirmAnswer(const FName& Answer);

	UFUNCTION(Reliable, Server, BlueprintCallable)
	void RequestNextRound();
	
	
	UFUNCTION(Reliable, Server, BlueprintCallable)
	void EndGame();
	
private:
	UFUNCTION()
	void OnOperatorDataReady(const FOperatorImage& Tex, const TArray<FString>& Hints);
	
	UFUNCTION()
	void OnGameEnd();
};
