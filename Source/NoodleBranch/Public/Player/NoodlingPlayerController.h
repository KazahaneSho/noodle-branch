// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonPlayerController.h"
#include "NoodlingPlayerController.generated.h"

#define UE_API NOODLEBRANCH_API

/**
 * Project Base PlayerController class.
 */
UCLASS(MinimalAPI, Config = Game)
class ANoodlingPlayerController : public ACommonPlayerController
{
	GENERATED_BODY()

public:

	UE_API explicit ANoodlingPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

// Player Controller for Replay Capture and Playback
UCLASS()
class ANoodlingReplayPlayerController : public ANoodlingPlayerController
{
	GENERATED_BODY()
};

#undef UE_API
