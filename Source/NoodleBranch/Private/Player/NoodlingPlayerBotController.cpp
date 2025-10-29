// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NoodlingPlayerBotController.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/PlayerState.h"
#include "GameModes/NoodlingGameMode.h"
#include "NoodleBranch/NoodlingLogChannels.h"
#include "Perception/AIPerceptionComponent.h"


ANoodlingPlayerBotController::ANoodlingPlayerBotController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsPlayerState = true;
	bStopAILogicOnUnposses = false;
}

void ANoodlingPlayerBotController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	UE_LOG(NoodleBranch_LogTeams,
		Error,
		TEXT("You can't set the team ID on a player bot controller (%s); it's driven by the associated player state"), *GetPathNameSafe(this));
}

FGenericTeamId ANoodlingPlayerBotController::GetGenericTeamId() const
{
	if (const INoodlingTeamAgentInterface* PlayerStateWithTeamInterface = Cast<INoodlingTeamAgentInterface>(PlayerState))
	{
		return PlayerStateWithTeamInterface->GetGenericTeamId();
	}
	return FGenericTeamId::NoTeam;
}

FOnNoodlingTeamIndexChangedDelegate* ANoodlingPlayerBotController::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

ETeamAttitude::Type ANoodlingPlayerBotController::GetTeamAttitudeTowards(const AActor& Other) const
{
	if (const APawn* OtherPawn = Cast<APawn>(&Other)) {

		if (const INoodlingTeamAgentInterface* TeamAgent = Cast<INoodlingTeamAgentInterface>(OtherPawn->GetController()))
		{
			//Checking Other pawn ID to define Attitude
			if (const FGenericTeamId OtherTeamID = TeamAgent->GetGenericTeamId(); OtherTeamID.GetId() != GetGenericTeamId().GetId())
			{
				return ETeamAttitude::Hostile;
			}
			else
			{
				return ETeamAttitude::Friendly;
			}
		}
	}

	return ETeamAttitude::Neutral;
}

void ANoodlingPlayerBotController::UpdateTeamAttitude(UAIPerceptionComponent* AIPerception)
{
	if (AIPerception)
	{
		AIPerception->RequestStimuliListenerUpdate();
	}
}

void ANoodlingPlayerBotController::OnUnPossess()
{
	// Make sure the pawn that is being unpossessed doesn't remain our AbilitySystemComponent's avatar actor.
	if (const APawn* PawnBeingUnpossessed = GetPawn())
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerState))
		{
			if (AbilitySystemComponent->GetAvatarActor() == PawnBeingUnpossessed)
			{
				AbilitySystemComponent->SetAvatarActor(nullptr);
			}
		}
	}
	
	Super::OnUnPossess();
}

void ANoodlingPlayerBotController::InitPlayerState()
{
	Super::InitPlayerState();
	BroadcastOnPlayerStateChanged();
}

void ANoodlingPlayerBotController::CleanupPlayerState()
{
	Super::CleanupPlayerState();
	BroadcastOnPlayerStateChanged();
}

void ANoodlingPlayerBotController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	BroadcastOnPlayerStateChanged();
}

void ANoodlingPlayerBotController::OnPlayerStateChanged()
{
	// Empty, place for derived classes to implement without having to hook all the other events
}

void ANoodlingPlayerBotController::OnPlayerStateChangedTeam(UObject* TeamAgent, const int32 OldTeam, const int32 NewTeam)
{
	ConditionalBroadcastTeamChanged(this,
		IntegerToGenericTeamId(OldTeam),
		IntegerToGenericTeamId(NewTeam));
}

void ANoodlingPlayerBotController::BroadcastOnPlayerStateChanged()
{
	OnPlayerStateChanged();

	// Unbind from the old player state, if any
	FGenericTeamId OldTeamID = FGenericTeamId::NoTeam;
	if (LastSeenPlayerState != nullptr)
	{
		if (INoodlingTeamAgentInterface* PlayerStateTeamInterface = Cast<INoodlingTeamAgentInterface>(LastSeenPlayerState))
		{
			OldTeamID = PlayerStateTeamInterface->GetGenericTeamId();
			PlayerStateTeamInterface->GetTeamChangedDelegateChecked().RemoveAll(this);
		}
	}

	// Bind to the new player state, if any
	FGenericTeamId NewTeamID = FGenericTeamId::NoTeam;
	if (PlayerState != nullptr)
	{
		if (INoodlingTeamAgentInterface* PlayerStateTeamInterface = Cast<INoodlingTeamAgentInterface>(PlayerState))
		{
			NewTeamID = PlayerStateTeamInterface->GetGenericTeamId();
			PlayerStateTeamInterface->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnPlayerStateChangedTeam);
		}
	}

	// Broadcast the team change (if it really has)
	ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);

	LastSeenPlayerState = PlayerState;
}

void ANoodlingPlayerBotController::ServerRestartController()
{
	if (GetNetMode() == NM_Client)
	{
		return;
	}

	ensure((GetPawn() == nullptr) && IsInState(NAME_Inactive));

	if (IsInState(NAME_Inactive) || (IsInState(NAME_Spectating)))
	{
		ANoodlingGameMode* const GameMode = GetWorld()->GetAuthGameMode<ANoodlingGameMode>();

		if ((GameMode == nullptr) || !GameMode->ControllerCanRestart(this))
		{
			return;
		}

		// If we're still attached to a Pawn, leave it
		if (GetPawn() != nullptr)
		{
			UnPossess();
		}

		// Re-enable input, similar to code in ClientRestart
		ResetIgnoreInputFlags();

		GameMode->RestartPlayer(this);
	}
}
