// Fill out your copyright notice in the Description page of Project Settings.


#include "GuessHudBase.h"

#include "OperatorNameObject.h"
#include "ArknightsGuess/Core/GuesserPlayerController.h"
#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "ArknightsGuess/Operators/OperatorSubsystem.h"
#include "ArknightsGuess/Operators/OperatorUISettings.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/ListView.h"


void UGuessHudBase::NativeConstruct()
{
	Super::NativeConstruct();
	if (auto Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->OnGuessRoundStateChanged.AddUniqueDynamic(this, &UGuessHudBase::OnGuessStateChanged);

		auto Names = Subsystem->GetAllOperatorNames();
		AllEntries.Reserve(Names.Num());
		for (const FName& ItElement : Names)
		{
			auto Object = NewObject<UOperatorNameObject>(this, ItElement);
			Object->Init(ItElement, OnPlayerInputAnswer).BindUObject(this, &UGuessHudBase::ConfirmFromList);
			AllEntries.Add(Object);
		}
	}
	
	ConfirmButton->OnClicked.AddDynamic(this, &UGuessHudBase::OnAnswerConfirmed);
	AnswerBox->OnTextChanged.AddDynamic(this, &UGuessHudBase::TryRetrieveAnswer);
	OperatorList->SetVisibility(ESlateVisibility::Collapsed);
	
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
	if (bConfirmedFromList)
	{
		bConfirmedFromList = false;
		return;
	}
	
	if (!OperatorList->IsVisible())
		OperatorList->SetVisibility(ESlateVisibility::Visible);
	
	OnPlayerInputAnswer.Broadcast(Text);

	TArray<UObject*> Matching;
	Matching.Reserve(AllEntries.Num());
	for (const auto& Entry : AllEntries)
	{
		if (Entry->GetShouldDisplay())
		{
			Matching.Add(Entry.Get());
		}
	}
	OperatorList->SetListItems(Matching);
	OperatorList->RegenerateAllEntries();
	

}

void UGuessHudBase::OnGuessStateChanged(EGuessRoundState State)
{
}

void UGuessHudBase::ConfirmFromList(const FName& Operator)
{
	bConfirmedFromList = true;
	OperatorList->SetVisibility(ESlateVisibility::Collapsed);
	AnswerBox->SetText(FText::FromName(Operator));
}
