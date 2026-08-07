// Fill out your copyright notice in the Description page of Project Settings.

#include "DevNotificationSubsystem.h"
#include "DevNotificationSettings.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"

void UDevNotificationSubsystem::Deinitialize()
{
	if (DismissTimer.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(DismissTimer);
		}
	}

	if (NotificationEntry)
	{
		NotificationEntry->RemoveFromParent();
		NotificationEntry = nullptr;
	}

	Super::Deinitialize();
}

void UDevNotificationSubsystem::SetEntryWidgetClass(TSubclassOf<UUserWidget> WidgetClass)
{
	EntryWidgetClass = WidgetClass;
}

void UDevNotificationSubsystem::ShowNotification(const FString& Body)
{
	UWorld* World = GetWorld();
	auto* Settings = UDevNotificationSettings::Get();
	if (!World || !Settings) return;

	// Clear previous dismiss timer
	if (DismissTimer.IsValid())
	{
		World->GetTimerManager().ClearTimer(DismissTimer);
	}

	// Fall back to settings default if no class was explicitly set
	if (!EntryWidgetClass)
	{
		EntryWidgetClass = Settings->DefaultEntryWidgetClass;
	}

	// Lazily create the entry widget once and reuse it
	if (!NotificationEntry && EntryWidgetClass)
	{
		NotificationEntry = CreateWidget<UUserWidget>(World, EntryWidgetClass);
		if (NotificationEntry)
		{
			NotificationEntry->AddToViewport();
		}
	}
	
	// Broadcast — UI binds here to drive the entry widget's text
	if (NotificationEntry)
	{
		NotificationEntry->RemoveFromParent();
		NotificationEntry->AddToViewport();
	}
	OnShowNotification.Broadcast(Body);

	// Auto-dismiss after settings duration
	World->GetTimerManager().SetTimer(DismissTimer, this,
		&UDevNotificationSubsystem::DismissEntry, Settings->DurationSeconds, false);
}

void UDevNotificationSubsystem::ShowNotificationTemplate(EDevNotificationTemplate Template)
{
	if (auto* Settings = UDevNotificationSettings::Get())
	{
		if (const FString* Text = Settings->TemplateTexts.Find(Template))
		{
			ShowNotification(*Text);
		}
	}
}

void UDevNotificationSubsystem::DismissEntry()
{
	OnDismissNotification.Broadcast();
}
