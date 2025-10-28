// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "NoodlingSettingsLocal.generated.h"

/**
 * UNoodlingSettingsLocal
 */
UCLASS()
class UNoodlingSettingsLocal : public UGameUserSettings
{
	GENERATED_BODY()

public:

	static UNoodlingSettingsLocal* Get();

	void OnExperienceLoaded();

	// Replay Settings
public:

	UFUNCTION()
	bool ShouldAutoRecordReplays() const { return bShouldAutoRecordReplays; }

private:

	UPROPERTY(Config)
	bool bShouldAutoRecordReplays = false;
};
