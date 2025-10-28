// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "NoodlingExperienceDefinition.generated.h"

class UNoodlingExperienceActionSet;
class UGameFeatureAction;
class UNoodlingPawnData;
/**
 * Definition of an experience.
 */
UCLASS(BlueprintType, Const)
class UNoodlingExperienceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UNoodlingExperienceDefinition();

	//~UObject interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~End of UObject interface

	//~UPrimaryDataAsset interface
#if WITH_EDITORONLY_DATA
	virtual void UpdateAssetBundleData() override;
#endif
	//~End of UPrimaryDataAsset interface

public:
	// List of Game Feature Plugins this experience wants to have active
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	TArray<FString> GameFeaturesToEnable;

	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	TSoftObjectPtr<const UNoodlingPawnData> DefaultPawnData;

	// List of actions to perform as this experience is loaded/activated/deactivated/unloaded
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Actions")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;
	
	// List of additional action sets to compose into this experience
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TArray<TObjectPtr<UNoodlingExperienceActionSet>> ActionSets;

};
