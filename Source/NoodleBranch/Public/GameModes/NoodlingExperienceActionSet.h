// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "NoodlingExperienceActionSet.generated.h"

class UGameFeatureAction;
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class UNoodlingExperienceActionSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

public:

	// List of actions to perform as this experience is loaded/activated/deactivated/unloaded
	UPROPERTY(EditAnywhere, Instanced, Category="Actions to Perform")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	// List of Game Feature Plugins this experience wants to have active
	UPROPERTY(EditAnywhere, Category="Feature Dependencies")
	TArray<FString> GameFeaturesToEnable;
};
