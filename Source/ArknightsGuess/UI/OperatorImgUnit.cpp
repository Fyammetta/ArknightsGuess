// Fill out your copyright notice in the Description page of Project Settings.


#include "OperatorImgUnit.h"

#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "ArknightsGuess/Operators/OperatorSubsystem.h"
#include "ArknightsGuess/Operators/OperatorTypes.h"
#include "Components/Image.h"

void UOperatorImgUnit::SetupUnit(const FOperatorData& Data)
{
	auto Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this);
	if (!Subsystem) return;
	Image->SetBrushFromMaterial(Subsystem->GetDynamicMaterial(Data));
}
