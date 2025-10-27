// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "NoodlingPawnData.generated.h"

#define UE_API NOODLEBRANCH_API

class UNoodlingCameraMode;
class UNoodlingInputConfig;
class UNoodlingAbilityTagRelationshipMapping;
class UNoodlingAbilitySet;
/**
 * Non-mutable data asset that contains properties defining a pawn.
 */
UCLASS(MinimalAPI, BlueprintType, Const, Meta = (DisplayName = "Noodling Pawn Data", ShortTooltip = "Data Asset used to define a Pawn"))
class UNoodlingPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	// Constructor called during initialization
	UE_API explicit UNoodlingPawnData(const FObjectInitializer& ObjectInitializer);

public:

	/**
	 * Properties that define a Pawn
	 */

	// Class to instantiate for this pawn.
	// This should usually be derived from ANoodlingPawn or ANoodlingCharacter
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NoodleBranch|Pawn")
	TSubclassOf<class ANoodlingPawnBase> PawnClass;

	// Ability sets to grant to this pawn's ability system.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NoodleBranch|Abilities")
	TArray<TObjectPtr<UNoodlingAbilitySet>> AbilitySets;

	// What mapping of ability tags to use for actions taking by this pawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NoodleBranch|Abilities")
	TObjectPtr<UNoodlingAbilityTagRelationshipMapping> TagRelationshipMapping;

	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NoodleBranch|Input")
	TObjectPtr<UNoodlingInputConfig> InputConfig;

	// Default camera mode used by player controlled pawns.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NoodleBranch|Camera")
	TSubclassOf<UNoodlingCameraMode> DefaultCameraMode;
	
};

#undef UE_API
