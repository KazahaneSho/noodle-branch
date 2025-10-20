// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularPlayerState.h"
#include "NoodlingPlayerState.generated.h"

#define UE_API NOODLEBRANCH_API

/**
 * Project Base PlayerState class.
 */
UCLASS()
class ANoodlingPlayerState : public AModularPlayerState
{
	GENERATED_BODY()

public:
	UE_API explicit ANoodlingPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

#undef UE_API
