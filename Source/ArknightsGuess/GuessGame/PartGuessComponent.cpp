// Fill out your copyright notice in the Description page of Project Settings.


#include "PartGuessComponent.h"


void UPartGuessComponent::OnNewRound()
{
}

void UPartGuessComponent::OnWrongGuess(int32 GuessCount)
{
}

// Sets default values for this component's properties
UPartGuessComponent::UPartGuessComponent()
{
	SetIsReplicatedByDefault(true);
}
