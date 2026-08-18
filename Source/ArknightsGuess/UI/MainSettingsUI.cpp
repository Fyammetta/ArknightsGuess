// Fill out your copyright notice in the Description page of Project Settings.


#include "MainSettingsUI.h"

#include "ArknightsGuess.h"
#include "Audio/GuessAudioSubsystem.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/TileView.h"
#include "Core/GuessGamerSettings.h"
#include "Core/GuessPlayerState.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Operators/OperatorTags.h"
#include "Operators/OperatorUISettings.h"
#include "UI/PlayerIconObject.h"

namespace
{
	bool FirstOpen = true;
}

void UMainSettingsUI::NativeConstruct()
{
	Super::NativeConstruct();
	if (!Resolutions.IsEmpty() && FirstOpen)
	{
		ResolutionsBox->SetScrollOffset(TextHeight * ResolutionIndex);

		return;
	}
	FirstOpen = false;
	auto Settings = UOperatorUISettings::Get();
	if (!Settings) return;
	
	TArray<UPlayerIconObject*> Objects;
	Settings->OperatorIcons.LoadSynchronous()->ForeachRow<FOperatorIconRow>(TEXT("[UMainSettingsUI] Foreach Icon "),
		[&Objects,this](const FName& RowName, const FOperatorIconRow& Row){
			auto Object = NewObject<UPlayerIconObject>(this, UPlayerIconObject::StaticClass(),RowName);
			Object->ChangePlayerIcon(Row.Icon.LoadSynchronous());
			Objects.Add(Object);
			Object->OnPlayerIconSelected.AddUObject(this, &UMainSettingsUI::OnSelectedIcon);
	});
	IconsTile->SetListItems(Objects);
	
	SaveButton->OnClicked.AddUniqueDynamic(this, &UMainSettingsUI::OnSaveClicked);
	QuitButton->OnClicked.AddUniqueDynamic(this, &UMainSettingsUI::OnQuitClicked);
	ConfirmIconButton->OnClicked.AddUniqueDynamic(this, &UMainSettingsUI::OnConfirmIconClicked);
	
	ResolutionUpButton->OnClicked.AddUniqueDynamic(this, &UMainSettingsUI::OnSetResolutionUp);
	ResolutionDownButton->OnClicked.AddUniqueDynamic(this, &UMainSettingsUI::OnSetResolutionDown);
	
	FullScreenButton->OnClicked.AddUniqueDynamic(this, &UMainSettingsUI::OnSetFullScreenMode);
	WindowedButton->OnClicked.AddUniqueDynamic(this, &UMainSettingsUI::OnSetWindowedMode);
	WindowedFullscreenButton->OnClicked.AddUniqueDynamic(this, &UMainSettingsUI::OnSetWindowedFullScreenMode);
	
	OnSelectedIcon(UGuessGamerSettings::GetPlayerIcon());
	auto Name = UGuessGamerSettings::GetPlayerName();
	if (!Name.IsEmpty())
	{
		PlayerNameInput->SetText(FText::FromString(Name));
	}

	
	MainVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UMainSettingsUI::OnMainVolumeChanged);
	UIVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UMainSettingsUI::OnUIVolumeChanged);
	VoiceVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UMainSettingsUI::OnVoiceVolumeChanged);
	MusicVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UMainSettingsUI::OnMusicVolumeChanged);

	InitVolumeSlider(MainVolumeSlider, SoundTags::Default());
	InitVolumeSlider(UIVolumeSlider, SoundTags::UI());
	InitVolumeSlider(VoiceVolumeSlider, SoundTags::Voice());
	InitVolumeSlider(MusicVolumeSlider, SoundTags::Music());
	
	GetAllAvailableResolutions();

}

void UMainSettingsUI::ClickSettingButton(UButton* Target)
{
	if (CurrentSettingsButton.IsValid())
	{
		CurrentSettingsButton->SetBackgroundColor(NormalSettingsColor);
	}
	
	if (Target)
	{
		Target->SetBackgroundColor(CurrentSettingsColor);
	}
	
	CurrentSettingsButton = Target;
}

void UMainSettingsUI::ClickScreenButton(UButton* Target)
{
	if (CurrentScreenButton.IsValid())
	{
		CurrentScreenButton->SetBackgroundColor(NormalSettingsColor);
	}
	
	if (Target)
	{
		Target->SetBackgroundColor(CurrentSettingsColor);
	}
	
	CurrentScreenButton = Target;
}

void UMainSettingsUI::OnSelectedIcon(UTexture2D* Icon)
{
	if (!Icon) return;
	auto Material = MainImage->GetDynamicMaterial();
	Material->SetTextureParameterValue(TEXT("Tex"), Icon);
	//MainImage->SetBrushFromTexture(Icon);
}

void UMainSettingsUI::OnConfirmIconClicked()
{
	auto Material = MainImage->GetDynamicMaterial();
	PlayerIcon = Cast<UTexture2D>(Material->K2_GetTextureParameterValue(TEXT("Tex")));
	if (!GetWorld() || !GetWorld()->GetFirstPlayerController()) return;
	
	if (!PlayerNameInput->GetText().IsEmpty())
	{
		PlayerName = PlayerNameInput->GetText().ToString();
	}
	
	if (auto PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<AGuessPlayerState>())
	{
		PS->ChangePlayerIcon(PlayerIcon);
		PS->SetPlayerName(PlayerName);
	}
}

void UMainSettingsUI::OnSaveClicked()
{
	
	
	UGuessGamerSettings::SetPlayerIcon(PlayerIcon);
	UGuessGamerSettings::SetPlayerName(PlayerName);

	for (const auto& Pair : SoundVolumeMapping)
	{
		UGuessGamerSettings::SetVolumeByTag(Pair.Key, Pair.Value);
	}
	if (!Resolutions.IsEmpty())
	{
		if (auto Settings = UGameUserSettings::GetGameUserSettings())
		{
			if (!Resolutions.IsEmpty())
			{
				if (ScreenMode == EWindowMode::Type::Fullscreen)
				{
					Settings->SetScreenResolution(Resolutions.Last());
					ResolutionIndex = Resolutions.Num() - 1;
				}
				else
				{
					ResolutionIndex = FMath::Clamp(ResolutionIndex, 0, Resolutions.Num() - 1);
					Settings->SetScreenResolution(Resolutions[ResolutionIndex]);
				}
			}
			Settings->SetFullscreenMode(ScreenMode);
		}
	}

	UGuessGamerSettings::Get()->ApplySettings(true);

	RemoveFromParent();
}

void UMainSettingsUI::OnQuitClicked()
{
	OnSelectedIcon(UGuessGamerSettings::GetPlayerIcon());
	auto Name = UGuessGamerSettings::GetPlayerName();
	if (!Name.IsEmpty())
	{
		PlayerNameInput->SetText(FText::FromString(Name));
	}

	InitVolumeSlider(MainVolumeSlider, SoundTags::Default());
	InitVolumeSlider(UIVolumeSlider, SoundTags::UI());
	InitVolumeSlider(VoiceVolumeSlider, SoundTags::Voice());
	InitVolumeSlider(MusicVolumeSlider, SoundTags::Music());
	RemoveFromParent();
}

void UMainSettingsUI::OnMainVolumeChanged(float Value)
{
	ApplyVolume(SoundTags::Default(), Value);
	
	MainVolumeText->SetText(FText::FromString(FString::FromInt(Value*100)));
}

void UMainSettingsUI::OnUIVolumeChanged(float Value)
{
	ApplyVolume(SoundTags::UI(), Value);
	
	UIVolumeText->SetText(FText::FromString(FString::FromInt(Value*100)));

}

void UMainSettingsUI::OnVoiceVolumeChanged(float Value)
{
	ApplyVolume(SoundTags::Voice(), Value);
	
	VoiceVolumeText->SetText(FText::FromString(FString::FromInt(Value*100)));

}

void UMainSettingsUI::OnMusicVolumeChanged(float Value)
{
	ApplyVolume(SoundTags::Music(), Value);
	
	MusicVolumeText->SetText(FText::FromString(FString::FromInt(Value*100)));

}

void UMainSettingsUI::ApplyVolume(const FGameplayTag& Tag, float Value)
{
	SoundVolumeMapping.FindOrAdd(Tag) = Value;

	if (auto* AudioSub = GetGameInstance()->GetSubsystem<UGuessAudioSubsystem>())
	{
		AudioSub->ApplyVolume(Tag, Value);
	}
}

void UMainSettingsUI::InitVolumeSlider(USlider* Slider, const FGameplayTag& Tag)
{
	if (!Slider) return;

	const float Value = UGuessGamerSettings::GetVolumeByTag(Tag);
	SoundVolumeMapping.Add(Tag, Value);
	Slider->SetValue(Value);
}

void UMainSettingsUI::GetAllAvailableResolutions()
{
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
	auto Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings) return;
	if (Resolutions.IsEmpty())
	{
		DisplaySettingsButton->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	ResolutionsBox->ClearChildren();
	ResolutionIndex = INDEX_NONE;
	
	auto Resolution = Settings->GetScreenResolution();

	for (const auto& Info : Resolutions)
	{
		auto Text = WidgetTree->ConstructWidget<UTextBlock>();
		auto Size = WidgetTree->ConstructWidget<USizeBox>();

		Text->SetText(FText::FromString(FString::Printf(TEXT("%d x %d"), Info.X, Info.Y)));
		Text->SetFont(ResolutionFont);
		Text->SetJustification(ETextJustify::Type::Center);
		Size->SetHeightOverride(TextHeight);
		
		Size->AddChild(Text);
		ResolutionsBox->AddChild(Size);
		
		if (Resolution == Info)
		{
			ResolutionIndex = Resolutions.Find(Resolution);
			ResolutionsBox->SetScrollOffset(ResolutionIndex * TextHeight);
		}
	}

	if (ResolutionIndex == INDEX_NONE)
	{
		ResolutionIndex = Resolutions.Num() / 2;
	}
	
		
	switch (UGuessGamerSettings::GetGameUserSettings()->GetFullscreenMode())
	{
	case EWindowMode::Type::Windowed:
		OnSetWindowedMode();
		break;
	case EWindowMode::Type::Fullscreen:
		OnSetFullScreenMode();
		break;
	case EWindowMode::Type::WindowedFullscreen:
		OnSetWindowedFullScreenMode();
		break;
	default:;
	}
}

namespace 
{
	constexpr float Rate = 0.01;
	constexpr float Alpha = 0.1;
	constexpr float Exp = 3;
}

void UMainSettingsUI::OnSetResolutionUp()
{
	if (ResolutionIndex < Resolutions.Num() - 1)
	{
		++ResolutionIndex;
		if (!ResolutionChangeTimer.IsValid())
		{
			GetWorld()->GetTimerManager().SetTimer(
			ResolutionChangeTimer, 
			FTimerDelegate::CreateUObject(this, &UMainSettingsUI::InterpResolutionOffset),
			Rate,
			true
			);
		}
	}

}

void UMainSettingsUI::OnSetResolutionDown()
{
	if (ResolutionIndex > 0)
	{
		--ResolutionIndex;
		if (!ResolutionChangeTimer.IsValid())
		{
			GetWorld()->GetTimerManager().SetTimer(
			ResolutionChangeTimer, 
			FTimerDelegate::CreateUObject(this, &UMainSettingsUI::InterpResolutionOffset),
			Rate,
			true
			);
		}
	}
}

void UMainSettingsUI::InterpResolutionOffset()
{
	float Cur = ResolutionsBox->GetScrollOffset();
	float Tar = ResolutionIndex * TextHeight;
	ResolutionsBox->SetScrollOffset(FMath::InterpEaseOut(Cur, Tar, Alpha, Exp));
	
	if (FMath::IsNearlyEqual(Cur, Tar))
	{
		ResolutionsBox->SetScrollOffset(Tar);
		GetWorld()->GetTimerManager().ClearTimer(ResolutionChangeTimer);
	}
}

void UMainSettingsUI::OnSetFullScreenMode()
{
	ScreenMode = EWindowMode::Type::Fullscreen;
	ClickScreenButton(FullScreenButton);
}

void UMainSettingsUI::OnSetWindowedMode()
{
	ScreenMode = EWindowMode::Type::Windowed;
	ClickScreenButton(WindowedButton);
}

void UMainSettingsUI::OnSetWindowedFullScreenMode()
{
	ScreenMode = EWindowMode::Type::WindowedFullscreen;
	ClickScreenButton(WindowedFullscreenButton);
}

