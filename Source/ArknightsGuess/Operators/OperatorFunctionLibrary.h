// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OperatorFunctionLibrary.generated.h"

class UOperatorSubsystem;
/**
 * 
 */
UCLASS()
class ARKNIGHTSGUESS_API UOperatorFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OperatorFunctionLibrary|Subsystem")
	static UOperatorSubsystem* GetOperatorSubsystem(UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OperatorFunctionLibrary|Gameplay")
	static void SetOperatorClarity(UMaterialInstanceDynamic* Material, int32 Clarity);
	
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "OperatorFunctionLibrary|Gameplay")
	static void SetOperatorDisplayPart(UMaterialInstanceDynamic* Material, const FVector& Detail);
};
