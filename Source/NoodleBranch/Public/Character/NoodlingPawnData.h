// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NoodlingPawnData.generated.h"

#define UE_API NOODLEBRANCH_API

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Noodling|Pawn")
	TSubclassOf<class ANoodlingPawn> PawnClass;
	
};

#undef UE_API
