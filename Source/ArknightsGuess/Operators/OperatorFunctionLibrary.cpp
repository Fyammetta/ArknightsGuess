// Fill out your copyright notice in the Description page of Project Settings.


#include "OperatorFunctionLibrary.h"
#include "OperatorSubsystem.h"
#include "Engine/GameInstance.h"

UOperatorSubsystem* UOperatorFunctionLibrary::GetOperatorSubsystem(UObject* WorldContextObject)
{
	if (const UWorld* World = WorldContextObject->GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<UOperatorSubsystem>();
		}
	}

	return nullptr;
}
