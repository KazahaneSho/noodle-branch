// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "NoodlingCheatManager.generated.h"

#ifndef USING_CHEAT_MANAGER
#define USING_CHEAT_MANAGER (1 && !UE_BUILD_SHIPPING)
#endif // #ifndef USING_CHEAT_MANAGER

DECLARE_LOG_CATEGORY_EXTERN(NoodleBranch_LogCheat, Log, All);

/**
 * 
 */
UCLASS(Config = Game, Within = PlayerController, MinimalAPI)
class UNoodlingCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:

	UNoodlingCheatManager();
};
