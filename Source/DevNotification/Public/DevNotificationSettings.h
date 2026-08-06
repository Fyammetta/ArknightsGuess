// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Blueprint/UserWidget.h"
#include "DevNotificationSettings.generated.h"

/** Preset notification templates for common dev messages. */
UENUM(BlueprintType)
enum class EDevNotificationTemplate : uint8
{
	UnderDevelopment	UMETA(DisplayName = "Under Development"),
	AbnormalBehavior	UMETA(DisplayName = "Abnormal Behavior"),
	NotImplemented		UMETA(DisplayName = "Not Implemented"),
	ComingSoon			UMETA(DisplayName = "Coming Soon"),
	Deprecated			UMETA(DisplayName = "Deprecated"),
};

/**
 * Configurable notification text for in-editor dev popups.
 * Edit in Project Settings → Game → Dev Notification.
 */
UCLASS(config=Game, DefaultConfig, DisplayName="Dev Notification")
class DEVNOTIFICATION_API UDevNotificationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static UDevNotificationSettings* Get()
	{
		return GetMutableDefault<UDevNotificationSettings>();
	}

	/** Template texts mapped to each preset notification type. */
	UPROPERTY(config, EditAnywhere, Category = "Templates")
	TMap<EDevNotificationTemplate, FString> TemplateTexts = {
		{ EDevNotificationTemplate::UnderDevelopment,	TEXT("This feature is under development.") },
		{ EDevNotificationTemplate::AbnormalBehavior,	TEXT("Abnormal behavior detected. Please check the logs.") },
		{ EDevNotificationTemplate::NotImplemented,		TEXT("This feature has not been implemented yet.") },
		{ EDevNotificationTemplate::ComingSoon,			TEXT("Coming soon. Stay tuned!") },
		{ EDevNotificationTemplate::Deprecated,			TEXT("This feature is deprecated and will be removed.") },
	};

	UPROPERTY(config, EditAnywhere, Category = "Notification", meta = (ClampMin = "0.5", ClampMax = "30.0"))
	float DurationSeconds = 5.0f;

	/** Default widget class used to create the notification entry. Can be overridden at runtime via SetEntryWidgetClass. */
	UPROPERTY(config, EditAnywhere, Category = "Notification", meta = (MetaClass = "/Script/UMG.UserWidget"))
	TSubclassOf<UUserWidget> DefaultEntryWidgetClass;
};
