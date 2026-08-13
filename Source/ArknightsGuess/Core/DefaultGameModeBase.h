// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DefaultGameModeBase.generated.h"

struct FGameplayTag;
/**
 * 
 */
UCLASS()
class ARKNIGHTSGUESS_API ADefaultGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	void StartGame(const FGameplayTag& Mode);

};
