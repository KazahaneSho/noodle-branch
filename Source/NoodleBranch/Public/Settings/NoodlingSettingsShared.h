// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "NoodlingSettingsShared.generated.h"

class UNoodlingLocalPlayer;
/**
 * UNoodlingSettingsShared - The "Shared" settings are stored as part of the USaveGame system, these settings are not machine
 * specific like the local settings, and are safe to store in the cloud - and 'share' them.  Using the save game system
 * we can also store settings per player, so things like controller keybind preferences should go here, because if those
 * are stored in the local settings all users would get them.
 */
UCLASS()
class UNoodlingSettingsShared : public ULocalPlayerSaveGame
{
	GENERATED_BODY()

public:
	DECLARE_EVENT_OneParam(UNoodlingSettingsShared, FOnSettingChangedEvent, const UNoodlingSettingsShared* Settings);
	FOnSettingChangedEvent OnSettingChanged;

public:

	UNoodlingSettingsShared();

	/** Creates a temporary settings object, this will be replaced by one loaded from the user's save game */
	static UNoodlingSettingsShared* CreateTemporarySettings(const UNoodlingLocalPlayer* LocalPlayer);
	
	/** Synchronously loads a settings object, this is not valid to call before login */
	static UNoodlingSettingsShared* LoadOrCreateSettings(const UNoodlingLocalPlayer* LocalPlayer);

	/** Applies the current settings to the player */
	void ApplySettings();

	////////////////////////////////////////////////////////
	// Gamepad Vibration
public:
	UFUNCTION()
	bool GetForceFeedbackEnabled() const { return bForceFeedbackEnabled; }

private:
	/** Is force feedback enabled when a controller is being used? */
	UPROPERTY()
	bool bForceFeedbackEnabled = true;

	////////////////////////////////////////////////////////
	/// Dirty and Change Reporting
private:
	template<typename T>
	bool ChangeValueAndDirty(T& CurrentValue, const T& NewValue)
	{
		if (CurrentValue != NewValue)
		{
			CurrentValue = NewValue;
			bIsDirty = true;
			OnSettingChanged.Broadcast(this);
			
			return true;
		}

		return false;
	}

	bool bIsDirty = false;
	
};
