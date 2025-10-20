// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "NoodlingPawnExtensionComponent.generated.h"

#define UE_API NOODLEBRANCH_API


/**
 * Component that adds functionality to all Pawn classes so it can be used for characters/vehicles/etc.
 * This coordinates the initialization of other components.
 */

class UNoodlingPawnData;

UCLASS(MinimalAPI)
class UNoodlingPawnExtensionComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UE_API explicit UNoodlingPawnExtensionComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "Noodling|Pawn")
	static UNoodlingPawnExtensionComponent* FindPawnExtensionComponent(const AActor* Actor) { return Actor ? Actor->FindComponentByClass<UNoodlingPawnExtensionComponent>() : nullptr; };
	
	// Set current pawn data
	UE_API void SetPawnData(const UNoodlingPawnData* InPawnData);

protected:
	virtual void BeginPlay() override;
};

#undef UE_API
