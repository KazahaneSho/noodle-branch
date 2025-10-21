// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "NoodlingVerbMessage.generated.h"
/**
 * Represents a generic message of the form Instigator Verb Target (in Context, with Magnitude)
 * Requires "GameplayTags" to be in the PublicDependencyModuleNames in Build.cs.
 */
USTRUCT(BlueprintType)
struct FNoodlingVerbMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTag Verb;
};
