// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerStatuesUI.h"

#include "AnswerUnit.h"
#include "GameFramework/PlayerState.h"
#include "Components/ListView.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Core/GuessGameStateBase.h"
#include "Core/GuessPlayerState.h"
#include "Operators/OperatorFunctionLibrary.h"
#include "Operators/OperatorSubsystem.h"


void UPlayerStatuesUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (auto GS = GetWorld() ? GetWorld()->GetGameState<AGuessGameStateBase>() : nullptr)
	{
		GS->WhenPlayerOnReady.AddUniqueDynamic(this,&UPlayerStatuesUI::OnPlayerReady);
		GS->OnPlayerAnswered.AddUObject(this, &UPlayerStatuesUI::OnOtherAnswered);
		
	}
	
	if (auto Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->OnGuessRoundStateChanged.AddUniqueDynamic(this, &UPlayerStatuesUI::OnGuessStateChanged);
	}

}



void UPlayerStatuesUI::OnPlayerReady(APlayerState* Player, bool bReady)
{
	if (!Player) return;
	
	if (bReady)
		PlayerList->AddItem(Player);
	else
		PlayerList->RemoveItem(Player);
}

void UPlayerStatuesUI::OnOtherAnswered(APlayerState* Player, const FName& Answer)
{
	if (!Player) return;
	
	if (auto PC = Player->GetPlayerController())
	{
		if (PC->IsLocalController())
			return;
	}
	
	if (AnswerWidget && Player->IsA(AGuessPlayerState::StaticClass()))
	{
		auto ChildSlot = AnswerBox->AddChildToVerticalBox(UAnswerUnit::MakeNewUnit(GetWorld(), AnswerWidget , Cast<AGuessPlayerState>(Player),Answer));
		ChildSlot->SetVerticalAlignment(VAlign_Bottom);
		ChildSlot->SetHorizontalAlignment(HAlign_Left);
	}
	
	
}

void UPlayerStatuesUI::OnGuessStateChanged(EGuessRoundState State)
{
}
