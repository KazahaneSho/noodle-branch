// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "NoodlingPlayerStart.generated.h"

#define UE_API NOODLEBRANCH_API

enum class ENoodlingPlayerStartLocationOccupancy
{
	Empty,
	Partial,
	Full
};

UCLASS(MinimalAPI, Config = Game)
class ANoodlingPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UE_API explicit ANoodlingPlayerStart(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UE_API ENoodlingPlayerStartLocationOccupancy GetLocationOccupancy(AController* const ControllerPawnToFit) const;

	/** Did this player start get claimed by a controller already? */
	UE_API bool IsClaimed() const;

	/** If this PlayerStart was not claimed, claim it for ClaimingController */
	UE_API bool TryClaim(AController* OccupyingController);

protected:
	/** Check if this PlayerStart is still claimed */
	UE_API void CheckUnclaimed();
	
	/** The controller that claimed this PlayerStart */
	UPROPERTY(Transient)
	TObjectPtr<AController> ClaimingController = nullptr;

	/** Interval in which we'll check if this player start is not colliding with anyone anymore */
	UPROPERTY(EditDefaultsOnly, Category = "Player Start Claiming")
	float ExpirationCheckInterval = 1.f;

	/** Handle to track expiration recurring timer */
	FTimerHandle ExpirationTimerHandle;
};

#undef UE_API
