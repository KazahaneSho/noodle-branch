// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NoodlingLocalPlayer.h"

#include "Settings/NoodlingSettingsLocal.h"
#include "Settings/NoodlingSettingsShared.h"

UNoodlingSettingsLocal* UNoodlingLocalPlayer::GetLocalSettings() const
{
	return UNoodlingSettingsLocal::Get();
}

UNoodlingSettingsShared* UNoodlingLocalPlayer::GetSharedSettings() const
{
	if (!SharedSettings)
	{
		// It's okay to use the sync load on PC, because it only checks the disk
		// This could use a platform tag to check for proper save support instead
		bool bCanLoadBeforeLogin = PLATFORM_DESKTOP;
		
		if (bCanLoadBeforeLogin)
		{
			SharedSettings = UNoodlingSettingsShared::LoadOrCreateSettings(this);
		}
		else
		{
			// We need to wait for user login to get the real settings so return temp ones
			SharedSettings = UNoodlingSettingsShared::CreateTemporarySettings(this);
		}
	}

	return SharedSettings;
}
