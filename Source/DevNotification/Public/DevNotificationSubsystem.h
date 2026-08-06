// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DevNotificationSettings.h"
#include "DevNotificationSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDevNotificationShowDelegate, const FString&, Body);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDevNotificationDismissDelegate);

/**
 * Manages a single reusable notification popup.
 * Only one notification exists at a time — new requests overwrite the previous one.
 */
UCLASS(BlueprintType)
class DEVNOTIFICATION_API UDevNotificationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	virtual void Deinitialize() override;

	/** Show notification with the given body text. Duration is read from UDevNotificationSettings. */
	UFUNCTION(BlueprintCallable)
	void ShowNotification(const FString& Body);

	/** Show a preset notification template (e.g. UnderDevelopment, AbnormalBehavior). */
	UFUNCTION(BlueprintCallable)
	void ShowNotificationTemplate(EDevNotificationTemplate Template);

	/** Set the widget class used to create the notification entry. Call once during init. */
	void SetEntryWidgetClass(TSubclassOf<UUserWidget> WidgetClass);

	/** UI binds here to update the entry widget's text when a notification fires. */
	UPROPERTY(BlueprintAssignable)
	FDevNotificationShowDelegate OnShowNotification;

	/** UI binds here to handle notification dismissal (e.g. fade-out, collapse). */
	UPROPERTY(BlueprintAssignable)
	FDevNotificationDismissDelegate OnDismissNotification;

private:
	void DismissEntry();

	UPROPERTY()
	TObjectPtr<UUserWidget> NotificationEntry;

	TSubclassOf<UUserWidget> EntryWidgetClass;

	FTimerHandle DismissTimer;
};
