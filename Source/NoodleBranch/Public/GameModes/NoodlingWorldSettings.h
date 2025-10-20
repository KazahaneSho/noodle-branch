// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "NoodlingWorldSettings.generated.h"

#define UE_API NOODLEBRANCH_API

/**
 * The default world settings object, used primarily to set the default gameplay experience to use when playing on this map
 */
UCLASS(MinimalAPI)
class ANoodlingWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:

	UE_API explicit ANoodlingWorldSettings(const FObjectInitializer& ObjectInitializer);
	// Returns the default experience to use when a server opens this map if it is not overridden by the NoodlingUserFacingExperienceDefinition.
	UE_API FPrimaryAssetId GetDefaultGameplayExperience() const;
};

#undef UE_API
