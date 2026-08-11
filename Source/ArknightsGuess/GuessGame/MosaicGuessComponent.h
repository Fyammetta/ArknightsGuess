// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UGuessComponentInterface.h"
#include "Components/ActorComponent.h"
#include "MosaicGuessComponent.generated.h"

/**
 * Mosaic-mode-specific state: random offset applied to the mosaic texture
 * for progressively revealing the operator image as the player makes wrong guesses.
 *
 * Owned by AGuessGameStateBase — lives and replicates with the GameState.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARKNIGHTSGUESS_API UMosaicGuessComponent : public UActorComponent, public IUGuessComponentInterface
{
	GENERATED_BODY()

public:
	UMosaicGuessComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ---- Accessor ----
	FVector2D GetMosaicOffset() const { return MosaicOffset; }
	void SetMosaicOffset(const FVector2D& Offset);

	// ---- IUGuessComponentInterface ----
	virtual void OnNewRound() override;
	virtual void OnWrongGuess(int32 GuessCount) override;

protected:
	UPROPERTY(ReplicatedUsing = "OnRep_MosaicOffset")
	FVector2D MosaicOffset;

	UFUNCTION()
	void OnRep_MosaicOffset();
};
