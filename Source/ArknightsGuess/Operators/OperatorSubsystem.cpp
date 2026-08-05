// Fill out your copyright notice in the Description page of Project Settings.


#include "OperatorSubsystem.h"

#include "OperatorTypes.h"
#include "OperatorUISettings.h"


void UOperatorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UnusedOperators.Empty();
	UsingOperators.Empty();

	OperatorData.LoadSynchronous()->ForeachRow<FOperatorDataRow>("",
		[&List = UnusedOperators](const FName& Key, const FOperatorDataRow& Row)->void
		{
			List.Append(FOperatorData::MakeFromDataRow(Key, Row));
		});
}

UMaterialInstanceDynamic* UOperatorSubsystem::GetDynamicMaterial(const FOperatorData& Data)
{
	auto Settings = UOperatorUISettings::Get();
	
	if (!Settings)
	{
		return nullptr;
	}
	
	if (!MaterialBase && Settings->Materials.Contains(GuessMode))
	{
		MaterialBase = UOperatorUISettings::Get()->Materials[GuessMode].LoadSynchronous();
	}
	
	if (!MaterialBase)
	{
		return nullptr;
	}
	
	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(MaterialBase, this);
	
	MID->SetTextureParameterValue(TEXT("Tex"),Data.Skin.LoadSynchronous());
	
	return MID;
}
