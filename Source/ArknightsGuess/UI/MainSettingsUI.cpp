// Fill out your copyright notice in the Description page of Project Settings.


#include "MainSettingsUI.h"
#include "Audio/GuessAudioSubsystem.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/TileView.h"
#include "Core/GuessGamerSettings.h"
#include "Core/GuessPlayerState.h"
#include "Operators/OperatorTags.h"
#include "Operators/OperatorUISettings.h"
#include "UI/PlayerIconObject.h"

void UMainSettingsUI::NativeConstruct()
{
	Super::NativeConstruct();
	
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
	QuitButton->OnClicked.AddUniqueDynamic(this, &UMainSettingsUI::RemoveFromParent);
	ConfirmIconButton->OnClicked.AddUniqueDynamic(this, &UMainSettingsUI::OnConfirmIconClicked);
	
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

	UGuessGamerSettings::Get()->SaveSettings();

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

