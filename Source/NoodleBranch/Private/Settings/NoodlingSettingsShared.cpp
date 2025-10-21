// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/NoodlingSettingsShared.h"

#include "Player/NoodlingLocalPlayer.h"

static FString SHARED_SETTINGS_SLOT_NAME = TEXT("SharedGameSettings");

namespace NoodlingSettingsSharedCVars
{
	static float DefaultGamepadLeftStickInnerDeadZone = 0.25f;
	static FAutoConsoleVariableRef CVarGamepadLeftStickInnerDeadZone(
		TEXT("gpad.DefaultLeftStickInnerDeadZone"),
		DefaultGamepadLeftStickInnerDeadZone,
		TEXT("Gamepad left stick inner dead-zone")
	);

	static float DefaultGamepadRightStickInnerDeadZone = 0.25f;
	static FAutoConsoleVariableRef CVarGamepadRightStickInnerDeadZone(
		TEXT("gpad.DefaultRightStickInnerDeadZone"),
		DefaultGamepadRightStickInnerDeadZone,
		TEXT("Gamepad right stick inner dead-zone")
	);	
}


UNoodlingSettingsShared::UNoodlingSettingsShared()
{
}

UNoodlingSettingsShared* UNoodlingSettingsShared::CreateTemporarySettings(const UNoodlingLocalPlayer* LocalPlayer)
{
	UNoodlingSettingsShared* SharedSettings = Cast<UNoodlingSettingsShared>(CreateNewSaveGameForLocalPlayer(UNoodlingSettingsShared::StaticClass(), LocalPlayer, SHARED_SETTINGS_SLOT_NAME));

	SharedSettings->ApplySettings();

	return SharedSettings;
}

UNoodlingSettingsShared* UNoodlingSettingsShared::LoadOrCreateSettings(const UNoodlingLocalPlayer* LocalPlayer)
{
	// This will stall the main thread while it loads
	UNoodlingSettingsShared* SharedSettings = Cast<UNoodlingSettingsShared>(LoadOrCreateSaveGameForLocalPlayer(UNoodlingSettingsShared::StaticClass(), LocalPlayer, SHARED_SETTINGS_SLOT_NAME));

	SharedSettings->ApplySettings();

	return SharedSettings;
}

void UNoodlingSettingsShared::ApplySettings()
{
}
