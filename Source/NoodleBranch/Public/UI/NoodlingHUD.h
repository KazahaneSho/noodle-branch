// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "NoodlingHUD.generated.h"

/**
 * 
 */
UCLASS(Config = Game)
class NOODLEBRANCH_API ANoodlingHUD : public AHUD
{
	GENERATED_BODY()

public:
	explicit ANoodlingHUD(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
