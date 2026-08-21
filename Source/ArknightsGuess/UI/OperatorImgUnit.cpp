// Fill out your copyright notice in the Description page of Project Settings.


#include "OperatorImgUnit.h"

#include "HintTextUnit.h"
#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "ArknightsGuess/Operators/OperatorSubsystem.h"
#include "ArknightsGuess/Operators/OperatorTypes.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/WidgetSwitcher.h"
#include "Core/GuesserPlayerController.h"


void UOperatorImgUnit::NativeConstruct()
{
	Super::NativeConstruct();
	
	Image->SetVisibility(ESlateVisibility::Collapsed);

	if (auto Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->OnOperatorDataReceived.AddUniqueDynamic(this, &UOperatorImgUnit::SetupUnit);
		Subsystem->OnGuessProcessChanged.AddUniqueDynamic(this, &UOperatorImgUnit::OnCallClarify);
		//Subsystem->OnGuessRoundStateChanged.AddUniqueDynamic(this, &UOperatorImgUnit::OnCheckAnswer);
		Subsystem->OnNextHintsDisplayAllowed.AddUniqueDynamic(this, &UOperatorImgUnit::OnShowNextHint);
		Subsystem->OnGuessRoundStateChanged.AddUniqueDynamic(this, &UOperatorImgUnit::OnGuessStateChanged);

	}
	
	Button_Cancel->OnClicked.AddUniqueDynamic(this, &UOperatorImgUnit::OnPlayerUnready);
	Button_Ready->OnClicked.AddUniqueDynamic(this, &UOperatorImgUnit::OnPlayerReady);
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

	CurrentHintIndex = 0;

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

	Image->SetVisibility(ESlateVisibility::Visible);
}

void UOperatorImgUnit::OnCallClarify(int32 Round, int32 Level)
{
	UOperatorFunctionLibrary::SetOperatorClarity(Image->GetDynamicMaterial(),Level);
}

/*void UOperatorImgUnit::OnCheckAnswer(EGuessRoundState RoundState)
{
	switch (RoundState)
	{
		case EGuessRoundState::Verify:  Image->GetDynamicMaterial()->SetScalarParameterValue(TEXT("Complete"), 1);
		case EGuessRoundState::Reveal:	UOperatorFunctionLibrary::SetOperatorClarity(Image->GetDynamicMaterial(),0);
		default:break;
	}
}*/

void UOperatorImgUnit::OnShowNextHint()
{
	const auto& Children = HintBox->GetAllChildren();
	if (CurrentHintIndex < Children.Num())
	{
		if (auto Hint = Cast<UHintTextUnit>(Children[CurrentHintIndex]))
		{
			Hint->SetVisibility(ESlateVisibility::Visible);
		}
		CurrentHintIndex++;
	}
}

void UOperatorImgUnit::OnPlayerReady()
{
	ButtonSwitcher->SetActiveWidgetIndex(1);

	if (auto PC = GetWorld() ? GetWorld()->GetFirstPlayerController<AGuesserPlayerController>() : nullptr)
		PC->RequestNextRound();
	
}

void UOperatorImgUnit::OnPlayerUnready()
{
	ButtonSwitcher->SetActiveWidgetIndex(0);

	if (auto PC = GetWorld() ? GetWorld()->GetFirstPlayerController<AGuesserPlayerController>() : nullptr)
		PC->CancelPreparedState();
	
}

void UOperatorImgUnit::OnGuessStateChanged(EGuessRoundState State)
{
	switch (State)
	{
		case EGuessRoundState::Verify:
		case EGuessRoundState::Reveal:
			UOperatorFunctionLibrary::SetOperatorClarity(Image->GetDynamicMaterial(),0);
			ButtonSwitcher->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			break;
		case EGuessRoundState::Guessing:
			ButtonSwitcher->SetActiveWidgetIndex(0); 
			ButtonSwitcher->SetVisibility(ESlateVisibility::Collapsed);
			break;
	}
}
