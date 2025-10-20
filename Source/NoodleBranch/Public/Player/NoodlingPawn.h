// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularPawn.h"
#include "NoodlingPawn.generated.h"

#define UE_API NOODLEBRANCH_API

UCLASS(MinimalAPI)
class ANoodlingPawn : public AModularPawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	UE_API explicit ANoodlingPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};

#undef UE_API
