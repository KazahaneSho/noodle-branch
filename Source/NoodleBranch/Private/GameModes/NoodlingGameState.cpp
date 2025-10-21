// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/NoodlingGameState.h"

#include "AbilitySystem/NoodlingAbilitySystemComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "GameModes/NoodlingExperienceManagerComponent.h"
#include "Messages/NoodlingVerbMessage.h"
#include "Net/UnrealNetwork.h"
#include "NoodleBranch/NoodlingLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NoodlingGameState)

extern ENGINE_API float GAverageFPS;

ANoodlingGameState::ANoodlingGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UNoodlingAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	ExperienceManagerComponent = CreateDefaultSubobject<UNoodlingExperienceManagerComponent>(TEXT("ExperienceManagerComponent"));

	ServerFPS = 0.0f;
}

void ANoodlingGameState::PreInitializeComponents()
{
	// No customization yet
	Super::PreInitializeComponents();
}

void ANoodlingGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(/*Owner=*/ this, /*Avatar=*/ this);
}

void ANoodlingGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ANoodlingGameState::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() == ROLE_Authority)
	{
		ServerFPS = GAverageFPS; // From extern ENGINE_API float GAverageFPS; at the top of this file.
	}
}

void ANoodlingGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
}

void ANoodlingGameState::RemovePlayerState(APlayerState* PlayerState)
{
	//@TODO: This isn't getting called right now (only the 'rich' AGameMode uses it, not AGameModeBase)
	// Need to at least comment the engine code, and possibly move things around
	Super::RemovePlayerState(PlayerState);
}

void ANoodlingGameState::SeamlessTravelTransitionCheckpoint(bool bToTransitionMap)
{
	// Remove inactive players and bots
	for (int32 i = PlayerArray.Num() - 1; i >= 0; i--)
	{
		if (APlayerState* PlayerState = PlayerArray[i]; PlayerState && (PlayerState->IsABot() || PlayerState->IsInactive()))
		{
			RemovePlayerState(PlayerState);
		}
	}
}

void ANoodlingGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ServerFPS);
	DOREPLIFETIME_CONDITION(ThisClass, RecorderPlayerState, COND_ReplayOnly);
}

UAbilitySystemComponent* ANoodlingGameState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

float ANoodlingGameState::GetServerFPS() const
{
	return ServerFPS;
}

void ANoodlingGameState::SetRecorderPlayerState(APlayerState* NewPlayerState)
{
	if (RecorderPlayerState == nullptr)
	{
		// Set it and call the rep callback so it can do any record-time setup
		RecorderPlayerState = NewPlayerState;
		OnRep_RecorderPlayerState();
	}
	else
	{
		UE_LOG(NoodleBranch_Log, Warning, TEXT("SetRecorderPlayerState was called on %s but should only be called once per game on the primary user"), *GetName());
	}
}

APlayerState* ANoodlingGameState::GetRecorderPlayerState() const
{
	// TODO: Maybe auto select it if null?

	return RecorderPlayerState;
}

void ANoodlingGameState::OnRep_RecorderPlayerState() const
{
	OnRecorderPlayerStateChangedEvent.Broadcast(RecorderPlayerState);
}

void ANoodlingGameState::MulticastReliableMessageToClients_Implementation(const FNoodlingVerbMessage Message)
{
	if (GetNetMode() == NM_Client)
	{
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Verb, Message);
	}
}

void ANoodlingGameState::MulticastMessageToClients_Implementation(const FNoodlingVerbMessage Message)
{
	MulticastMessageToClients_Implementation(Message);
}
