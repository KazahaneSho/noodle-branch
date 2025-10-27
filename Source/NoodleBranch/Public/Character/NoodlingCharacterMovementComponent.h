// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NoodlingCharacterMovementComponent.generated.h"

#define UE_API NOODLEBRANCH_API

UCLASS(MinimalAPI, Config=Game)
class UNoodlingCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UNoodlingCharacterMovementComponent();
	
	UE_API void SetReplicatedAcceleration(const FVector& InAcceleration);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};

#undef UE_API
