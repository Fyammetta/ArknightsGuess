// Fill out your copyright notice in the Description page of Project Settings.


#include "GuessHudBase.h"
#include "Animation/UMGSequencePlayer.h"
#include "OperatorNameObject.h"
#include "ArknightsGuess/Core/GuesserPlayerController.h"
#include "ArknightsGuess/Operators/OperatorFunctionLibrary.h"
#include "ArknightsGuess/Operators/OperatorSubsystem.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/ListView.h"
#include "DevNotification/Public/DevNotificationSubsystem.h"
#include "ArknightsGuess.h"


void UGuessHudBase::NativeConstruct()
{
	Super::NativeConstruct();

	// Ensure the widget receives mouse events
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	// Clear stale entries from a previous game session
	// (NativeConstruct fires again when the cached widget is re-added to viewport)
	AllEntries.Empty();

	if (auto Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->OnGuessRoundStateChanged.AddUniqueDynamic(this, &UGuessHudBase::OnGuessStateChanged);

		auto Names = Subsystem->GetAllOperatorNames();
		AllEntries.Reserve(Names.Num());
		for (const auto& ItElement : Names)
		{
			auto Object = NewObject<UOperatorNameObject>(this, ItElement.RealName);
			Object->Init(ItElement, OnPlayerInputAnswer).BindUObject(this, &UGuessHudBase::ConfirmFromList);
			AllEntries.Add(Object);
		}
	}
	
	ConfirmButton->OnClicked.AddUniqueDynamic(this, &UGuessHudBase::OnAnswerConfirmed);
	QuitButton->OnClicked.AddUniqueDynamic(this, &UGuessHudBase::OnTryingQuit);
	ConfirmQuitButton->OnClicked.AddUniqueDynamic(this, &UGuessHudBase::OnQuitConfirmed);
	CancelQuitButton->OnClicked.AddUniqueDynamic(this, &UGuessHudBase::OnQuitCanceled);
	MusicSettingButton->OnClicked.AddUniqueDynamic(this, &UGuessHudBase::OnMusicSettingClicked);
	AnswerBox->OnTextChanged.AddUniqueDynamic(this, &UGuessHudBase::TryRetrieveAnswer);
	DisplayAllButton->OnClicked.AddUniqueDynamic(this, &UGuessHudBase::ShowAllOperators);
	OperatorList->SetVisibility(ESlateVisibility::Collapsed);
	bPreparedForNext = true;
}

void UGuessHudBase::NativeDestruct()
{
	if (auto Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		Subsystem->OnGuessRoundStateChanged.RemoveDynamic(this, &UGuessHudBase::OnGuessStateChanged);
	}

	Super::NativeDestruct();
}

FReply UGuessHudBase::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bTryingQuit)
	{
		if (QuitUI->GetCachedGeometry().IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
			return FReply::Unhandled();
		
		OnQuitCanceled();
		return FReply::Handled();
	}
	
	if (bPreparedForNext)
	{
		if (auto PC = GetWorld() ? GetWorld()->GetFirstPlayerController<AGuesserPlayerController>() : nullptr)
			PC->RequestNextRound();
	}
	
	return FReply::Unhandled();
}

void UGuessHudBase::OnMusicSettingClicked()
{
	if (auto Subsystem = GetGameInstance()->GetSubsystem<UDevNotificationSubsystem>())
	{
		Subsystem->ShowNotificationTemplate(EDevNotificationTemplate::UnderDevelopment);
	}
}

void UGuessHudBase::OnAnswerConfirmed()
{
	UE_LOG(LogArknights, Log, TEXT("[HUD] OnAnswerConfirmed | Text=%s"), *AnswerBox->GetText().ToString());

	// 输入框内容为空时，弹出提示并阻止确认
	const FString TrimmedAnswer = AnswerBox->GetText().ToString().TrimStartAndEnd();

	if (TrimmedAnswer.IsEmpty())
	{
		UE_LOG(LogArknights, Warning, TEXT("[HUD] OnAnswerConfirmed blocked: empty input"));
		if (auto Notification = GetGameInstance()->GetSubsystem<UDevNotificationSubsystem>())
		{
			Notification->ShowNotification(TEXT("请先输入干员名称再确认"));
		}
		return;
	}
	
	if (auto Subsystem = UOperatorFunctionLibrary::GetOperatorSubsystem(this))
	{
		UE_LOG(LogArknights, Warning, TEXT("[HUD] OnAnswerConfirmed blocked: illegal input"));

		if (!Subsystem->GetAllOperatorNames().Contains(FOperatorNamePair(FName(TrimmedAnswer),{})))
		{
			if (auto Notification = GetGameInstance()->GetSubsystem<UDevNotificationSubsystem>())
			{
				Notification->ShowNotification(TEXT("请先输入完整的干员名称或点击选项后再确认"));
			}
			return;
		}
	}


	auto PC = GetWorld() ? GetWorld()->GetFirstPlayerController<AGuesserPlayerController>() : nullptr;
	auto Answer = FName(AnswerBox->GetText().ToString());
	if (!PC) { UE_LOG(LogArknights, Warning, TEXT("[HUD] OnAnswerConfirmed failed: no PlayerController")); return; }

	OperatorList->SetVisibility(ESlateVisibility::Collapsed);
	AnswerBox->SetText(FText::GetEmpty());
	PC->ConfirmAnswer(Answer);
}

void UGuessHudBase::OnTryingQuit()
{
	UE_LOG(LogArknights, Log, TEXT("[HUD] OnTryingQuit"));
	PlayAnimationForward(CallQuitUI);
	bTryingQuit = true;
}

void UGuessHudBase::OnQuitConfirmed()
{
	UE_LOG(LogArknights, Log, TEXT("[HUD] OnQuitConfirmed -> EndGame"));
	TWeakObjectPtr<AGuesserPlayerController> PC = GetWorld() ? GetWorld()->GetFirstPlayerController<AGuesserPlayerController>() : nullptr;
	PlayAnimation(CallQuitUI,1,1,EUMGSequencePlayMode::Reverse)->OnSequenceFinishedPlaying().AddWeakLambda(this,
		[PC](UUMGSequencePlayer&)
		{		
			if (PC.IsValid())
				PC->EndGame();
		});
}

void UGuessHudBase::OnQuitCanceled()
{
	UE_LOG(LogArknights, Log, TEXT("[HUD] OnQuitCanceled"));
	PlayAnimationReverse(CallQuitUI);
	bTryingQuit = false;

}

void UGuessHudBase::ShowAllOperators()
{
	if (OperatorList->IsVisible())
	{
		OperatorList->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		OperatorList->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		OnPlayerInputAnswer.Broadcast(FText::FromString(TEXT("*")));
		RefreshOperatorList();
	}
}

void UGuessHudBase::TryRetrieveAnswer(const FText& Text)
{
	if (!OperatorList->IsVisible())
		OperatorList->SetVisibility(ESlateVisibility::Visible);
	
	OnPlayerInputAnswer.Broadcast(Text);

	RefreshOperatorList();
}

void UGuessHudBase::RefreshOperatorList()
{
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
}

void UGuessHudBase::OnGuessStateChanged(EGuessRoundState State)
{
	UE_LOG(LogArknights, Log, TEXT("[HUD] OnGuessStateChanged | State=%d"), static_cast<int32>(State));
	bPreparedForNext = State == EGuessRoundState::Reveal || State == EGuessRoundState::Verify;
}

void UGuessHudBase::ConfirmFromList(const FName& Operator)
{
	AnswerBox->SetText(FText::FromName(Operator));

	OperatorList->SetVisibility(ESlateVisibility::Collapsed);
}
