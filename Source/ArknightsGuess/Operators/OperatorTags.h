// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

// ---- Game tags (function-local static to avoid init-order issues) ----

#define ARKNIGHTS_DECLARE_TAG(FuncName, TagString) \
	inline const FGameplayTag& FuncName() { static const FGameplayTag T = FGameplayTag::RequestGameplayTag(TagString); return T; }

namespace GameModeTags
{
	ARKNIGHTS_DECLARE_TAG(Root,   "GameMode")
	ARKNIGHTS_DECLARE_TAG(Mosaic, "GameMode.Mosaic")
	ARKNIGHTS_DECLARE_TAG(Part,   "GameMode.Part")
}

namespace SettingTags
{
	ARKNIGHTS_DECLARE_TAG(DefaultLevel,  "Settings.DefaultLevel")
	ARKNIGHTS_DECLARE_TAG(ShuffleLimit,  "Settings.ShuffleLimit")
	ARKNIGHTS_DECLARE_TAG(MaxGuessCount, "Settings.MaxGuessCount")
	ARKNIGHTS_DECLARE_TAG(HintFrequency, "Settings.HintFrequency")
}

namespace SoundTags
{
	ARKNIGHTS_DECLARE_TAG(Default, "Audio.Default")
	ARKNIGHTS_DECLARE_TAG(UI,      "Audio.UI")
	ARKNIGHTS_DECLARE_TAG(Voice,   "Audio.Voice")
	ARKNIGHTS_DECLARE_TAG(Music,   "Audio.Music")
}

namespace UITags
{
	ARKNIGHTS_DECLARE_TAG(Loading,   "Main.Loading")
	ARKNIGHTS_DECLARE_TAG(MainEntry,  "Main.MainEntry")
	ARKNIGHTS_DECLARE_TAG(Settings,   "Main.Settings")
	ARKNIGHTS_DECLARE_TAG(GuessGame,  "Game.GuessGame")
	ARKNIGHTS_DECLARE_TAG(Waiting,  "Main.Waiting")
	ARKNIGHTS_DECLARE_TAG(MultiRoom,   "Game.Multiplay")
}

namespace MapTags
{
	ARKNIGHTS_DECLARE_TAG(MultiRoom,   "Game.Multiplay")
	ARKNIGHTS_DECLARE_TAG(Main,   "Game.Multiplay")
}

#undef ARKNIGHTS_DECLARE_TAG
