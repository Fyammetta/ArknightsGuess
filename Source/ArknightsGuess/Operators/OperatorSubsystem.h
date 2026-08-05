// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OperatorSubsystem.generated.h"

struct FOperatorData;
/**
 * 
 */
UCLASS()
class ARKNIGHTSGUESS_API UOperatorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	UPROPERTY()
	TSoftObjectPtr<UDataTable> OperatorData;
	
	UPROPERTY()
	TArray<FOperatorData> UnusedOperators;
	
	UPROPERTY()
	TArray<FOperatorData> UsingOperators;
	
	UPROPERTY()
	UMaterial* MaterialBase;
	
	FName GuessMode;
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
public:
	UMaterialInstanceDynamic* GetDynamicMaterial(const FOperatorData& Data);
};
