// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NoodlingReplaySubsystem.generated.h"

#define UE_API NOODLEBRANCH_API

/**
 * Subsystem to handle recording/loading replays
 */
UCLASS(MinimalAPI)
class UNoodlingReplaySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UE_API UNoodlingReplaySubsystem();

	/** Starts recording a client replay, and handles any file cleanup needed */
	UFUNCTION(BlueprintCallable, Category = Replays)
	UE_API void RecordClientReplay(APlayerController* PlayerController);
};

#undef UE_API