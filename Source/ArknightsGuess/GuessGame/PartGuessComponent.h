// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UGuessComponentInterface.h"
#include "Components/ActorComponent.h"
#include "PartGuessComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARKNIGHTSGUESS_API UPartGuessComponent : public UActorComponent, public IUGuessComponentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	virtual void OnNewRound() override;
	virtual void OnWrongGuess(int32 GuessCount) override;
	UPartGuessComponent();
};
