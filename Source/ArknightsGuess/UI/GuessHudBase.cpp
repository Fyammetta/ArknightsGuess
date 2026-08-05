// Fill out your copyright notice in the Description page of Project Settings.


#include "GuessHudBase.h"

#include "ArknightsGuess/Core/GuesserPlayerController.h"
#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "ArknightsGuess/Operators/OperatorSubsystem.h"
#include "ArknightsGuess/Operators/OperatorUISettings.h"
#include "Components/Button.h"
#include "Components/EditableText.h"



void UGuessHudBase::NativeConstruct()
{
	Super::NativeConstruct();
	if (auto Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->OnGuessRoundStateChanged.AddUniqueDynamic(this, &UGuessHudBase::OnGuessStateChanged);
	}
	
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UGuessHudBase::OnAnswerConfirmed);
	}
	if (AnswerBox)
	{
		AnswerBox->OnTextChanged.AddDynamic(this, &UGuessHudBase::TryRetrieveAnswer);
	}
}

void UGuessHudBase::OnAnswerConfirmed() 
{
	auto PC = GetWorld() ? GetWorld()->GetFirstPlayerController<AGuesserPlayerController>() : nullptr;
	auto Answer = FName(AnswerBox->GetText().ToString());
	if (!PC) return;
	
	PC->ConfirmAnswer(Answer);
}

void UGuessHudBase::TryRetrieveAnswer(const FText& Text)
{
	
}

void UGuessHudBase::OnGuessStateChanged(EGuessRoundState State)
{
}
