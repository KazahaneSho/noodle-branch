// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/NoodlingPawnBase.h"

#include "Net/UnrealNetwork.h"
#include "NoodleBranch/NoodlingLogChannels.h"


ANoodlingPawnBase::ANoodlingPawnBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ANoodlingPawnBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, MyTeamID);
}

void ANoodlingPawnBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void ANoodlingPawnBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ANoodlingPawnBase::PossessedBy(AController* NewController)
{
	const FGenericTeamId OldTeamID = MyTeamID;
	
	Super::PossessedBy(NewController);

	// Grab the current team ID and listen for future changes
	if (INoodlingTeamAgentInterface* ControllerAsTeamProvider = Cast<INoodlingTeamAgentInterface>(NewController))
	{
		MyTeamID = ControllerAsTeamProvider->GetGenericTeamId();
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
	}
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void ANoodlingPawnBase::UnPossessed()
{
	AController* const OldController = GetController();

	// Stop listening for changes from the old controller
	const FGenericTeamId OldTeamID = MyTeamID;
	if (INoodlingTeamAgentInterface* ControllerAsTeamProvider = Cast<INoodlingTeamAgentInterface>(OldController))
	{
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
	}

	Super::UnPossessed();

	// Determine what the new team ID should be afterward
	MyTeamID = DetermineNewTeamAfterPossessionEnds(OldTeamID);
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void ANoodlingPawnBase::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	if (GetController() == nullptr)
	{
		if (HasAuthority())
		{
			const FGenericTeamId OldTeamID = MyTeamID;
			MyTeamID = NewTeamID;
			ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
		}
		else
		{
			UE_LOG(NoodleBranch_LogTeams, Error, TEXT("You can't set the team ID on a pawn (%s) without HasAuthority()"), *GetPathNameSafe(this));
		}
	}
	else
	{
		UE_LOG(NoodleBranch_LogTeams, Error, TEXT("You can't set the team ID on a possessed pawn (%s); it's driven by the associated controller"), *GetPathNameSafe(this));
	}
}

FGenericTeamId ANoodlingPawnBase::GetGenericTeamId() const
{
	return MyTeamID;
}

FOnNoodlingTeamIndexChangedDelegate* ANoodlingPawnBase::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

void ANoodlingPawnBase::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, const int32 NewTeam)
{
	const FGenericTeamId MyOldTeamID = MyTeamID;
	MyTeamID = IntegerToGenericTeamId(NewTeam);
	ConditionalBroadcastTeamChanged(this, MyOldTeamID, MyTeamID);
}

void ANoodlingPawnBase::OnRep_MyTeamID(const FGenericTeamId OldTeamID)
{
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}



