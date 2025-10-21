// Fill out your copyright notice in the Description page of Project Settings.


#include "Teams/NoodlingTeamAgentInterface.h"

#include "NoodleBranch/NoodlingLogChannels.h"
#include "UObject/ScriptInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NoodlingTeamAgentInterface)

// TODO: See if GENERATED_BODY() affects this Interface vs GENERATED_UINTERFACE_BODY() + GENERATED_IINTERFACE_BODY()
// and a basic constructor needs to be implemented here.

void INoodlingTeamAgentInterface::ConditionalBroadcastTeamChanged(const TScriptInterface<INoodlingTeamAgentInterface>& This, const FGenericTeamId OldTeamID, const FGenericTeamId NewTeamID)
{
	if (OldTeamID != NewTeamID)
	{
		const int32 OldTeamIndex = GenericTeamIdToInteger(OldTeamID); 
		const int32 NewTeamIndex = GenericTeamIdToInteger(NewTeamID);

		UObject* ThisObj = This.GetObject();
		UE_LOG(NoodleBranch_LogTeams, Verbose, TEXT("[%s] %s assigned team %d"), *GetClientServerContextString(ThisObj), *GetPathNameSafe(ThisObj), NewTeamIndex);

		This.GetInterface()->GetTeamChangedDelegateChecked().Broadcast(ThisObj, OldTeamIndex, NewTeamIndex);
	}
}
