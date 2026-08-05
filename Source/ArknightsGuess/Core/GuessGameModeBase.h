// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArknightsGuess/Operators/OperatorTypes.h"
#include "GameFramework/GameMode.h"
#include "GuessGameModeBase.generated.h"

class AGuessGameStateBase;
class UOperatorSubsystem;

UCLASS()
class ARKNIGHTSGUESS_API AGuessGameModeBase : public AGameMode
{
	GENERATED_BODY()

	UPROPERTY()
	FOperatorData CorrectAnswer;
protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> MainUIClass;
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UUserWidget> MainUIWidget;

public:
	virtual void BeginPlay() override;

	// ---- Round flow (server-only, called by PlayerController RPCs) ----
	void StartNewRound();
	void ProcessGuess(const FName& OperatorName);

protected:
	void SetRoundState(EGuessRoundState NewState) const;
	UOperatorSubsystem* GetOperatorSubsystem() const;
};
