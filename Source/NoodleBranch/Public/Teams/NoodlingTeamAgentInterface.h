// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "UObject/Interface.h"
#include "NoodlingTeamAgentInterface.generated.h"

#define UE_API NOODLEBRANCH_API

template <typename InterfaceType> class TScriptInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnNoodlingTeamIndexChangedDelegate, UObject*, ObjectChangingTeam, int32, OldTeamID, int32, NewTeamID);

inline int32 GenericTeamIdToInteger(const FGenericTeamId ID)
{
	return (ID == FGenericTeamId::NoTeam) ? INDEX_NONE : static_cast<int32>(ID);
}

inline FGenericTeamId IntegerToGenericTeamId(const int32 ID)
{
	return (ID == INDEX_NONE) ? FGenericTeamId::NoTeam : FGenericTeamId(static_cast<uint8>(ID));
}

// Interface for actors which can be associated with teams
UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UNoodlingTeamAgentInterface : public UGenericTeamAgentInterface // Requires PublicDependencyModule "AIModule" in Build.cs.
{
	GENERATED_BODY()
};

/**
 * 
 */
class INoodlingTeamAgentInterface : public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:

	virtual FOnNoodlingTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() { return nullptr; }

	static UE_API void ConditionalBroadcastTeamChanged(const TScriptInterface<INoodlingTeamAgentInterface>& This, const FGenericTeamId OldTeamID, const FGenericTeamId NewTeamID);
	
	FOnNoodlingTeamIndexChangedDelegate& GetTeamChangedDelegateChecked()
	{
		FOnNoodlingTeamIndexChangedDelegate* Result = GetOnTeamIndexChangedDelegate();
		check(Result);
		return *Result;
	}
};

#undef UE_API
