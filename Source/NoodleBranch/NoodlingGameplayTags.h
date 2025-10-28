// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

namespace NoodlingGameplayTags
{
	// Declare all the custom native tags that NoodleBranch will use.
	
	NOODLEBRANCH_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Behavior_SurvivesDeath);

	NOODLEBRANCH_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_Spawned);
	NOODLEBRANCH_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataAvailable);
	NOODLEBRANCH_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataInitialized);
	NOODLEBRANCH_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_GameplayReady);
	
	NOODLEBRANCH_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Crouching);
	NOODLEBRANCH_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_AutoRunning);	

	// These are mappings from MovementMode enums to GameplayTags associated with those enums (below)
	NOODLEBRANCH_API  extern const TMap<uint8, FGameplayTag> MovementModeTagMap;
	NOODLEBRANCH_API  extern const TMap<uint8, FGameplayTag> CustomMovementModeTagMap;

	NOODLEBRANCH_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Walking);
	NOODLEBRANCH_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_NavWalking);
	NOODLEBRANCH_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Falling);
	NOODLEBRANCH_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Swimming);
	NOODLEBRANCH_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Flying);

	NOODLEBRANCH_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Custom);
}
