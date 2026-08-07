// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "UIManagerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIShown, FGameplayTag, Tag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIHidden, FGameplayTag, Tag);

/**
 * Centralized UI manager.
 * Lazy-loads widget classes from UUIManagerSettings::UIRegistry on first use,
 * caches active instances, and provides push/pop-style lifetime control.
 */
UCLASS()
class ARKNIGHTSGUESS_API UUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ---- Subsystem lifetime ----
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Convenience accessor. */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "UIManager")
	static UUIManagerSubsystem* Get(const UObject* WorldContextObject);

	// ---- UI lifecycle API ----

	/**
	 * Lazy-loads and displays the widget registered under Tag.
	 * If already visible, re-orders to top (or no-op if same).
	 * Returns the widget instance, or nullptr on failure.
	 */
	UFUNCTION(BlueprintCallable, Category = "UIManager")
	UUserWidget* ShowUI(const FGameplayTag& Tag);

	/** Hides the widget by removing it from the viewport. Does NOT destroy the instance. */
	UFUNCTION(BlueprintCallable, Category = "UIManager")
	void HideUI(const FGameplayTag& Tag);

	/** Hides and destroys the cached instance. Next ShowUI will re-create it. */
	UFUNCTION(BlueprintCallable, Category = "UIManager")
	void DestroyUI(const FGameplayTag& Tag);

	/** Returns the cached widget for Tag, or nullptr if never created / already destroyed. */
	UFUNCTION(BlueprintCallable, Category = "UIManager")
	UUserWidget* GetUI(const FGameplayTag& Tag) const;

	/** Whether a widget is currently active (created and visible). */
	UFUNCTION(BlueprintCallable, Category = "UIManager")
	bool IsUIShown(const FGameplayTag& Tag) const;

	/** Hide all active UIs. */
	UFUNCTION(BlueprintCallable, Category = "UIManager")
	void HideAllUI();

	/** Hide and destroy all active UIs. */
	UFUNCTION(BlueprintCallable, Category = "UIManager")
	void DestroyAllUI();

	// ---- Delegates ----
	UPROPERTY(BlueprintAssignable, Category = "UIManager|Delegates")
	FOnUIShown OnUIShown;

	UPROPERTY(BlueprintAssignable, Category = "UIManager|Delegates")
	FOnUIHidden OnUIHidden;

private:
	/** Cached active widget instances, keyed by tag. */
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UUserWidget>> ActiveUIs;
};
