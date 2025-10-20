// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ModularGameState.h"
#include "WorldPartition/ActorDescContainerSubsystem.h"
#include "NoodlingGameState.generated.h"

#define UE_API NOODLEBRANCH_API

/** 
 * Project Base GameState class.
 */
UCLASS(MinimalAPI, Config = Game)
class ANoodlingGameState : public AModularGameStateBase
{
	GENERATED_BODY()

	public:

		UE_API explicit ANoodlingGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

		//~AActor interface
		UE_API virtual void PostInitializeComponents() override;
		UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
		//~End of AActor interface

		//~AGameStateBase interface
		UE_API virtual void SeamlessTravelTransitionCheckpoint(bool bToTransitionMap) override;
		//~End of AGameStateBase interface
};

#undef UE_API
