// Fill out your copyright notice in the Description page of Project Settings.


#include "OperatorFunctionLibrary.h"
#include "OperatorSubsystem.h"
#include "OperatorUISettings.h"
#include "ArknightsGuess.h"
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
	UE_LOG(LogArknights, Log, TEXT("[Lib] SetOperatorClarity | Clarity=%d"), Clarity);
	if (!Material || Clarity < 0) return;

	const int32 Step = UOperatorUISettings::Get()->ClarityPerLevel;

	if (Clarity % Step == 0 && Clarity != 0)
	{
		Material->SetScalarParameterValue(TEXT("OffsetX"),FMath::RandRange(-Step,Step));
		Material->SetScalarParameterValue(TEXT("OffsetY"),FMath::RandRange(-Step,Step));
	}
	Material->SetScalarParameterValue(TEXT("Level"), Clarity / Step);
	Material->SetScalarParameterValue(TEXT("SubLevel"), Clarity % Step);
}

void UOperatorFunctionLibrary::SetOperatorDisplayPart(UMaterialInstanceDynamic* Material, const FVector& Detail)
{
	UE_LOG(LogArknights, Log, TEXT("[Lib] SetOperatorDisplayPart | Detail=(%.2f, %.2f, %.2f)"), Detail.X, Detail.Y, Detail.Z);
	if (!Material || Detail.Z <= 0) return;

	Material->SetScalarParameterValue(TEXT("X"),Detail.X);
	Material->SetScalarParameterValue(TEXT("Y"),Detail.Y);
	Material->SetScalarParameterValue(TEXT("Multiply"),Detail.Z);
}
