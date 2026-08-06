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

void UOperatorFunctionLibrary::SetOperatorClarity(UMaterialInstanceDynamic* Material, int32 Clarity)
{
	if (!Material || Clarity < 0) return;
	
	if (Clarity % 4 == 0 && Clarity != 0)
	{
		Material->SetScalarParameterValue(TEXT("OffsetX"),FMath::RandRange(-4,4));
		Material->SetScalarParameterValue(TEXT("OffsetY"),FMath::RandRange(-4,4));
	}
	Material->SetScalarParameterValue(TEXT("Level"), Clarity / 4);
	Material->SetScalarParameterValue(TEXT("SubLevel"), Clarity % 4);
}

void UOperatorFunctionLibrary::SetOperatorDisplayPart(UMaterialInstanceDynamic* Material, const FVector& Detail)
{
	if (!Material || Detail.Z <= 0) return;

	Material->SetScalarParameterValue(TEXT("X"),Detail.X);
	Material->SetScalarParameterValue(TEXT("Y"),Detail.Y);
	Material->SetScalarParameterValue(TEXT("Multiply"),Detail.Z);
}
