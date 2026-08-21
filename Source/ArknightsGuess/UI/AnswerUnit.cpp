// Fill out your copyright notice in the Description page of Project Settings.


#include "AnswerUnit.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Core/GuessPlayerState.h"


void UAnswerUnit::NativeConstruct()
{
	Super::NativeConstruct();
	
	CloseButton->OnClicked.AddUniqueDynamic(this, &UAnswerUnit::RemoveFromParent);
}

UAnswerUnit* UAnswerUnit::MakeNewUnit(UWorld* Outer, TSubclassOf<UAnswerUnit> Class, AGuessPlayerState* Player, const FName& Answer)
{
	auto Widget = CreateWidget<UAnswerUnit>(Outer, Class);
	
	Widget->Text_Answer->SetText(FText::FromName(Answer));
	Widget->Text_PlayerName->SetText(FText::FromString(Player->GetPlayerName()));
	Widget->Icon->GetDynamicMaterial()->SetTextureParameterValue(TEXT("Tex"), Player->GetPlayerIcon());
	return Widget;
}
