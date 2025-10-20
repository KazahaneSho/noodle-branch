// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "NoodlingGameSession.generated.h"

UCLASS(Config = Game)
class NOODLEBRANCH_API ANoodlingGameSession : public AGameSession
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	explicit ANoodlingGameSession(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
