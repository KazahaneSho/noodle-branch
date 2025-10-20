// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "NoodlingDeveloperSettings.generated.h"

class UNoodlingExperienceDefinition;

/**
 * Developer settings / Editor cheats.
 */
UCLASS(Config = EditorPerProjectUserSettings, MinimalAPI)
class UNoodlingDeveloperSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:

	UNoodlingDeveloperSettings();

public:

	// The experience override to use for Play in Editor (if not set, the default for the world settings of the open map will be used)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category=NoodleBranch, meta=(AllowedTypes="NoodlingExperienceDefinition"))
	FPrimaryAssetId ExperienceOverride;
};
