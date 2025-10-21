// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "NoodlingAbilitySystemComponent.generated.h"

#define UE_API NOODLEBRANCH_API

/*
 * Requires "GameplayAbilities", "GameplayTags", and "GameplayTasks" to be in the PublicDependencyModuleNames in Build.cs.
 * Also Requires Enabling in Editor first Edit > Plugins and check Gameplay Abilities.
 */

UCLASS(MinimalAPI)
class UNoodlingAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	UE_API explicit UNoodlingAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API void TryActivateAbilitiesOnSpawn();

	UE_API void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
};

#undef UE_API