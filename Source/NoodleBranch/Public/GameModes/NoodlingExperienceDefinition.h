// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NoodlingExperienceDefinition.generated.h"

class UNoodlingPawnData;
/**
 * Definition of an experience.
 */
UCLASS(BlueprintType, Const)
class NOODLEBRANCH_API UNoodlingExperienceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UNoodlingExperienceDefinition();

public:

	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	TSoftObjectPtr<const UNoodlingPawnData> DefaultPawnData;
};
