// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularAIController.h"
#include "NoodlingPlayerBotController.generated.h"

/**
 * ANoodlingBotController
 *
 * The AI controller class for the project.
 */
UCLASS(Blueprintable)
class ANoodlingPlayerBotController : public AModularAIController
{
	GENERATED_BODY()

public:
	explicit ANoodlingPlayerBotController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Attempt to restart this controller (e.g. To respawn it)
	void ServerRestartController();

protected:
};
