// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemComponent.h"
#include "NoodlingAbilitySystemComponent.generated.h"

#define UE_API NOODLEBRANCH_API

/*
 * Requires "GameplayAbilities", "GameplayTags", and "GameplayTasks" to be in the PublicDependencyModuleNames in Build.cs.
 * Also Requires Enabling in Editor first Edit > Plugins and check Gameplay Abilities.
 */

class UNoodlingAbilityTagRelationshipMapping;

UCLASS(MinimalAPI)
class UNoodlingAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	UE_API explicit UNoodlingAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API void TryActivateAbilitiesOnSpawn();

	UE_API void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	UE_API void ClearAbilityInput();
	
	/** Sets the current tag relationship mapping, if null it will clear it out */
	UE_API void SetTagRelationshipMapping(UNoodlingAbilityTagRelationshipMapping* NewMapping);
};

#undef UE_API