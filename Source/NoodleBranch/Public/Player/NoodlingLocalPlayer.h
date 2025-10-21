// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonLocalPlayer.h"
#include "Teams/NoodlingTeamAgentInterface.h"
#include "NoodlingLocalPlayer.generated.h"

#define UE_API NOODLEBRANCH_API

class UNoodlingSettingsLocal;
class UNoodlingSettingsShared;
/**
 * 
 */
UCLASS(MinimalAPI)
class UNoodlingLocalPlayer : public UCommonLocalPlayer, public INoodlingTeamAgentInterface
{
	GENERATED_BODY()

public:

	/** Gets the local settings for this player, this is read from config files at process startup and is always valid */
	UFUNCTION()
	UE_API UNoodlingSettingsLocal* GetLocalSettings() const;

	/** Gets the shared setting for this player, this is read using the save game system so may not be correct until after user login */
	UFUNCTION()
	UE_API UNoodlingSettingsShared* GetSharedSettings() const;

private:

	UPROPERTY(Transient)
	mutable TObjectPtr<UNoodlingSettingsShared> SharedSettings;

};
