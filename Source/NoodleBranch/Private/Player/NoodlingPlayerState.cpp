// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NoodlingPlayerState.h"

#include "AbilitySystem/NoodlingAbilitySet.h"
#include "AbilitySystem/NoodlingAbilitySystemComponent.h"
#include "Attributes/NoodlingCombatSet.h"
#include "Attributes/NoodlingHealthSet.h"
#include "Character/NoodlingPawnData.h"
#include "Character/NoodlingPawnExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameModes/NoodlingExperienceManagerComponent.h"
#include "GameModes/NoodlingGameMode.h"
#include "Messages/NoodlingVerbMessage.h"
#include "Net/UnrealNetwork.h"
#include "NoodleBranch/NoodlingLogChannels.h"
#include "Player/NoodlingPlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NoodlingPlayerState)

const FName ANoodlingPlayerState::NAME_NoodlingAbilityReady("NoodlingAbilitiesReady");

//@TODO: Would like to isolate this a bit better to get the pawn data in here without this having to know about other stuff

ANoodlingPlayerState::ANoodlingPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), MyPlayerConnectionType(ENoodlingPlayerConnectionType::Player)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UNoodlingAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// These attribute sets will be detected by AbilitySystemComponent::InitializeComponent. Keeping a reference so that the sets don't get garbage collected before that.
	HealthSet = CreateDefaultSubobject<UNoodlingHealthSet>(TEXT("HealthSet"));
	CombatSet = CreateDefaultSubobject<UNoodlingCombatSet>(TEXT("CombatSet"));

	// AbilitySystemComponent needs to be updated at a high frequency.
	SetNetUpdateFrequency(100.0f);

	MyTeamID = FGenericTeamId::NoTeam;
	MySquadID = INDEX_NONE;
}

void ANoodlingPlayerState::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void ANoodlingPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent)
	AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());

	if (const UWorld* World = GetWorld(); World && World->IsGameWorld() && World->GetNetMode() != NM_Client)
	{
		const AGameStateBase* GameState = GetWorld()->GetGameState();
		check(GameState);
		UNoodlingExperienceManagerComponent* ExperienceManagerComponent = GameState->FindComponentByClass<UNoodlingExperienceManagerComponent>();
		check(ExperienceManagerComponent);
		ExperienceManagerComponent->CallOrRegister_OnExperienceLoaded(FOnNoodlingExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
	}
}

void ANoodlingPlayerState::Reset()
{
	Super::Reset();
}

void ANoodlingPlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);

	if (UNoodlingPawnExtensionComponent* PawnExtensionComponent = UNoodlingPawnExtensionComponent::FindPawnExtensionComponent(GetPawn()))
	{
		PawnExtensionComponent->CheckDefaultInitialization();
	}
}

void ANoodlingPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	//@TODO: Copy stats
}

void ANoodlingPlayerState::OnDeactivated()
{
	bool bDestroyDeactivatedPlayerState = false;

	switch (GetPlayerConnectionType())
	{
	case ENoodlingPlayerConnectionType::Player:
	case ENoodlingPlayerConnectionType::InactivePlayer:
		//@TODO: Ask the experience if we should destroy disconnecting players immediately or leave them around
		// (e.g., for long running servers where they might build up if lots of players cycle through)
		bDestroyDeactivatedPlayerState = true;
		break;
	default:
		bDestroyDeactivatedPlayerState = true;
		break;
	}
	
	SetPlayerConnectionType(ENoodlingPlayerConnectionType::InactivePlayer);

	if (bDestroyDeactivatedPlayerState)
	{
		Destroy();
	}
}

void ANoodlingPlayerState::OnReactivated()
{
	if (GetPlayerConnectionType() == ENoodlingPlayerConnectionType::InactivePlayer)
	{
		SetPlayerConnectionType(ENoodlingPlayerConnectionType::Player);
	}
}

void ANoodlingPlayerState::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	if (HasAuthority())
	{
		const FGenericTeamId OldTeamId = MyTeamID;

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MyTeamID, this);
		MyTeamID = NewTeamID;
		ConditionalBroadcastTeamChanged(this, OldTeamId, NewTeamID);
	}
	else
	{
		UE_LOG(NoodleBranch_LogTeams,
			Error,
			TEXT("Cannot set team for %s without HasAuthority()"), *GetPathName(this));
	}
}

FGenericTeamId ANoodlingPlayerState::GetGenericTeamId() const
{
	return MyTeamID;
}

FOnNoodlingTeamIndexChangedDelegate* ANoodlingPlayerState::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

ANoodlingPlayerController* ANoodlingPlayerState::GetNoodlingPlayerController() const
{
	return Cast<ANoodlingPlayerController>(GetOwner());
}

UAbilitySystemComponent* ANoodlingPlayerState::GetAbilitySystemComponent() const
{
	return GetNoodlingAbilitySystemComponent();
}

void ANoodlingPlayerState::SetPawnData(const UNoodlingPawnData* InPawnData)
{
	check(InPawnData);

	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (PawnData)
	{
		UE_LOG(NoodleBranch_Log,
			Error,
			TEXT("Trying to set PawnData [%s] on player state [%s] that already has valid PawnData [%s]."),
			*GetNameSafe(InPawnData), *GetNameSafe(this), *GetNameSafe(PawnData));
		return;
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
	PawnData = InPawnData;

	for (const UNoodlingAbilitySet* AbilitySet : PawnData->AbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
		}
	}

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_NoodlingAbilityReady);

	ForceNetUpdate();
}

void ANoodlingPlayerState::SetPlayerConnectionType(const ENoodlingPlayerConnectionType NewType)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MyPlayerConnectionType, this);
	MyPlayerConnectionType = NewType;
}

void ANoodlingPlayerState::SetSquadID(const int32 NewSquadId)
{
	if (HasAuthority())
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MySquadID, this);

		MySquadID = NewSquadId;
	}
}

void ANoodlingPlayerState::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.AddStack(Tag, StackCount);
}

void ANoodlingPlayerState::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.RemoveStack(Tag, StackCount);
}

int32 ANoodlingPlayerState::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTags.GetStackCount(Tag);
}

bool ANoodlingPlayerState::HasStatTag(FGameplayTag Tag) const
{
	return StatTags.ContainsTag(Tag);
}

FRotator ANoodlingPlayerState::GetReplicatedViewRotation() const
{
	// Could replace this with custom replication
	return ReplicatedViewRotation;
}

void ANoodlingPlayerState::SetReplicatedViewRotation(const FRotator& NewRotation)
{
	if (NewRotation != ReplicatedViewRotation)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ReplicatedViewRotation, this);
		ReplicatedViewRotation = NewRotation;
	}
}

void ANoodlingPlayerState::ClientBroadcastMessage_Implementation(const FNoodlingVerbMessage Message)
{
	// This check is needed to prevent running the action when in standalone mode
	if (GetNetMode() == NM_Client)
	{
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Verb, Message);
	}
}

void ANoodlingPlayerState::OnRep_PawnData()
{
}

void ANoodlingPlayerState::OnExperienceLoaded(const UNoodlingExperienceDefinition* CurrentExperience)
{
	if (const ANoodlingGameMode* NoodlingGameMode = GetWorld()->GetAuthGameMode<ANoodlingGameMode>())
	{
		if (const UNoodlingPawnData* NewPawnData = NoodlingGameMode->GetPawnDataForController(GetOwningController()))
		{
			SetPawnData(NewPawnData);
		}
		else
		{
			UE_LOG(NoodleBranch_Log,
				Error,
				TEXT("ANoodlingPlayerState::OnExperienceLoaded(): Unable to find PawnData to initialize player state [%s]!"),
				*GetNameSafe(this));
		}
	}
}

void ANoodlingPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyPlayerConnectionType, SharedParams)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyTeamID, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MySquadID, SharedParams);

	SharedParams.Condition = ELifetimeCondition::COND_SkipOwner;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ReplicatedViewRotation, SharedParams);

	DOREPLIFETIME(ThisClass, StatTags);	
}

void ANoodlingPlayerState::OnRep_MyTeamID(const FGenericTeamId OldTeamID)
{
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void ANoodlingPlayerState::OnRep_MySquadID()
{
	//@TODO: Let the squad subsystem know (once that exists)
}
