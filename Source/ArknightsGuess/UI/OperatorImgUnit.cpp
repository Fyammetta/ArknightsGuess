// Fill out your copyright notice in the Description page of Project Settings.


#include "OperatorImgUnit.h"

#include "HintTextUnit.h"
#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "ArknightsGuess/Operators/OperatorSubsystem.h"
#include "ArknightsGuess/Operators/OperatorTypes.h"
#include "Components/Image.h"
#include "Components/WrapBox.h"


void UOperatorImgUnit::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (auto Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->OnOperatorDataReceived.AddUniqueDynamic(this, &UOperatorImgUnit::SetupUnit);
		Subsystem->OnGuessProcessChanged.AddUniqueDynamic(this, &UOperatorImgUnit::OnCallClarify);
		Subsystem->OnGuessRoundStateChanged.AddUniqueDynamic(this, &UOperatorImgUnit::OnCheckAnswer);
	}
}

void UOperatorImgUnit::SetupUnit(const FOperatorImage& Img, const TArray<FString>& Hints)
{
	auto Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this);
	if (!Subsystem) return;
	auto Dynamic = Subsystem->GetDynamicMaterial();
	auto Tex = Img.Texture.LoadSynchronous();
	Dynamic->SetTextureParameterValue(TEXT("Tex"),Tex);
	UOperatorFunctionLibrary::SetOperatorClarity(Dynamic,Subsystem->GetDefaultLevel());
	Image->SetBrushFromMaterial(Dynamic);
	const auto& Children = HintBox->GetAllChildren();
	for (int i = 0; i < Children.Num(); ++i)
	{
		if (auto Hint = Cast<UHintTextUnit>(Children[i]))
		{		
			Hint->SetVisibility(ESlateVisibility::Collapsed);

			if (Hints.IsValidIndex(i))
				Hint->SetUpTextUnit(Hints[i]);
		}
	}

}

void UOperatorImgUnit::OnCallClarify(int32 Round, int32 Level)
{
	UOperatorFunctionLibrary::SetOperatorClarity(Image->GetDynamicMaterial(),Level);
}

void UOperatorImgUnit::OnCheckAnswer(EGuessRoundState RoundState)
{
	switch (RoundState)
	{
		case EGuessRoundState::Verify:Image->GetDynamicMaterial()->SetScalarParameterValue(TEXT("Complete"), 1);
		case EGuessRoundState::Reveal:	UOperatorFunctionLibrary::SetOperatorClarity(Image->GetDynamicMaterial(),0);
		default:break;
	}
}
