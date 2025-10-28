// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/NoodlingPawnExtensionComponent.h"

#include "AbilitySystem/NoodlingAbilitySystemComponent.h"
#include "Character/NoodlingPawnData.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Net/UnrealNetwork.h"
#include "NoodleBranch/NoodlingGameplayTags.h"
#include "NoodleBranch/NoodlingLogChannels.h"

UNoodlingPawnExtensionComponent::UNoodlingPawnExtensionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);	

	PawnData = nullptr;
	AbilitySystemComponent = nullptr;
}

const FName UNoodlingPawnExtensionComponent::NAME_ActorFeatureName("PawnExtension");

void UNoodlingPawnExtensionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UNoodlingPawnExtensionComponent, PawnData);
}

bool UNoodlingPawnExtensionComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager,
	const FGameplayTag CurrentState, const FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();
	if (!CurrentState.IsValid() && DesiredState == NoodlingGameplayTags::InitState_Spawned)
	{
		// As long as we are on a valid pawn, we count as spawned
		if (Pawn)
		{
			return true;
		}
	}
	if (CurrentState == NoodlingGameplayTags::InitState_Spawned
		&& DesiredState == NoodlingGameplayTags::InitState_DataAvailable)
	{
		// Pawn data is required.
		if (!PawnData)
		{
			return false;
		}

		if (Pawn->HasAuthority() || Pawn->IsLocallyControlled())
		{
			// Check for being possessed by a controller.
			if (!GetController<AController>())
			{
				return false;
			}
		}

		return true;
	}
	if (CurrentState == NoodlingGameplayTags::InitState_DataAvailable
		&& DesiredState == NoodlingGameplayTags::InitState_DataInitialized)
	{
		// Transition to initialize if all features have their data available
		return Manager->HaveAllFeaturesReachedInitState(Pawn, NoodlingGameplayTags::InitState_DataAvailable);
	}
	if (CurrentState == NoodlingGameplayTags::InitState_DataInitialized
		&& DesiredState == NoodlingGameplayTags::InitState_GameplayReady)
	{
		return true;
	}
	
	// We don't have Matching InitState_ GameplayTags
	return false;
}

void UNoodlingPawnExtensionComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager,
	FGameplayTag CurrentState, const FGameplayTag DesiredState)
{
	if (DesiredState == NoodlingGameplayTags::InitState_DataInitialized)
	{
		// This is currently all handled by other components listening to this state change
	}
}

void UNoodlingPawnExtensionComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// If another feature is now in DataAvailable, see if we should transition to DataInitialized
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		if (Params.FeatureState == NoodlingGameplayTags::InitState_DataAvailable)
		{
			CheckDefaultInitialization();
		}
	}
}

void UNoodlingPawnExtensionComponent::CheckDefaultInitialization()
{
	// Before checking our progress, try progressing any other features we might depend on
	CheckDefaultInitializationForImplementers();

	static const TArray<FGameplayTag> StateChain =
		{ NoodlingGameplayTags::InitState_Spawned,
		NoodlingGameplayTags::InitState_DataAvailable,
		NoodlingGameplayTags::InitState_DataInitialized,
		NoodlingGameplayTags::InitState_GameplayReady };

	// This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
	ContinueInitStateChain(StateChain);
}

void UNoodlingPawnExtensionComponent::OnRegister()
{
	Super::OnRegister();
	
	const APawn* Pawn = GetPawn<APawn>();
	ensureAlwaysMsgf((Pawn != nullptr),
		TEXT("NoodlingPawnExtensionComponent on [%s] can only be added to Pawn actors."),
		*GetNameSafe(GetOwner()));

	TArray<UActorComponent*> PawnExtensionComponents;
	Pawn->GetComponents(ThisClass::StaticClass(), PawnExtensionComponents);
	ensureAlwaysMsgf((PawnExtensionComponents.Num() == 1),
		TEXT("Only one NoodlingPawnExtensionComponent should exist on [%s]."),
		*GetNameSafe(GetOwner()));

	// Register with the init state system early, this will only work if this is a game world
	RegisterInitStateFeature();
}

void UNoodlingPawnExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnInitializeAbilitySystem();
	UnregisterInitStateFeature();
	
	Super::EndPlay(EndPlayReason);
}

void UNoodlingPawnExtensionComponent::OnRep_PawnData()
{
	CheckDefaultInitialization();
}

void UNoodlingPawnExtensionComponent::SetPawnData(const UNoodlingPawnData* InPawnData)
{
	check(InPawnData);

	APawn* Pawn = GetPawnChecked<APawn>();

	if (Pawn->GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (PawnData)
	{
		UE_LOG(NoodleBranch_Log, Error,
		       TEXT("Trying to set PawnData [%s] on pawn [%s] that already has valid PawnData [%s]."),
		       *GetNameSafe(InPawnData), *GetNameSafe(Pawn), *GetNameSafe(PawnData));
		return;
	}

	PawnData = InPawnData;

	Pawn->ForceNetUpdate();

	CheckDefaultInitialization();
}

void UNoodlingPawnExtensionComponent::OnAbilitySystemInitialized_RegisterAndCall(
	const FSimpleMulticastDelegate::FDelegate& Delegate)
{
	if (!OnAbilitySystemInitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemInitialized.Add(Delegate);
	}

	if (AbilitySystemComponent)
	{
		Delegate.Execute();
	}
}

void UNoodlingPawnExtensionComponent::OnAbilitySystemUninitialized_Register(
	const FSimpleMulticastDelegate::FDelegate& Delegate)
{
	if (!OnAbilitySystemUninitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemUninitialized.Add(Delegate);
	}
}

void UNoodlingPawnExtensionComponent::InitializeAbilitySystem(UNoodlingAbilitySystemComponent* InAbilitySystemComponent,
	AActor* InOwnerActor)
{
	check(InAbilitySystemComponent);
	check(InOwnerActor);

	if (AbilitySystemComponent == InAbilitySystemComponent)
	{
		// The ability system component hasn't changed.
		return;
	}

	if (AbilitySystemComponent)
	{
		// Clean up the old ability system component.
		UnInitializeAbilitySystem();
	}

	APawn* Pawn = GetPawnChecked<APawn>();
	const AActor* ExistingAvatar = InAbilitySystemComponent->GetAvatarActor();

	UE_LOG(NoodleBranch_Log, Verbose, TEXT("Setting up ASC [%s] on pawn [%s] owner [%s], existing [%s] "),
		*GetNameSafe(InAbilitySystemComponent), *GetNameSafe(Pawn), *GetNameSafe(InOwnerActor), *GetNameSafe(ExistingAvatar));

	if ((ExistingAvatar != nullptr) && (ExistingAvatar != Pawn))
	{
		UE_LOG(NoodleBranch_Log, Log, TEXT("Existing avatar (authority=%d)"), ExistingAvatar->HasAuthority() ? 1 : 0);

		// There is already a pawn acting as the ASC's avatar, so we need to kick it out
		// This can happen on clients if they're lagged: their new pawn is spawned + possessed before the dead one is removed
		ensure(!ExistingAvatar->HasAuthority());

		if (UNoodlingPawnExtensionComponent* OtherExtensionComponent = FindPawnExtensionComponent(ExistingAvatar))
		{
			OtherExtensionComponent->UnInitializeAbilitySystem();
		}
	}

	AbilitySystemComponent = InAbilitySystemComponent;
	AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, Pawn);

	if (ensure(PawnData))
	{
		InAbilitySystemComponent->SetTagRelationshipMapping(PawnData->TagRelationshipMapping);
	}

	OnAbilitySystemInitialized.Broadcast();
}

void UNoodlingPawnExtensionComponent::UnInitializeAbilitySystem()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
	if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
	{
		FGameplayTagContainer AbilityTypesToIgnore;
		AbilityTypesToIgnore.AddTag(NoodlingGameplayTags::Ability_Behavior_SurvivesDeath);

		AbilitySystemComponent->CancelAbilities(nullptr, &AbilityTypesToIgnore);
		AbilitySystemComponent->ClearAbilityInput();
		AbilitySystemComponent->RemoveAllGameplayCues();

		if (AbilitySystemComponent->GetOwnerActor() != nullptr)
		{
			AbilitySystemComponent->SetAvatarActor(nullptr);
		}
		else
		{
			// If the ASC doesn't have a valid owner, we need to clear *all* actor info, not just the avatar pairing
			AbilitySystemComponent->ClearActorInfo();
		}

		OnAbilitySystemUninitialized.Broadcast();
	}

	AbilitySystemComponent = nullptr;
}

void UNoodlingPawnExtensionComponent::HandleControllerChanged()
{
	if (AbilitySystemComponent && (AbilitySystemComponent->GetAvatarActor() == GetPawnChecked<APawn>()))
	{
		ensure(AbilitySystemComponent->AbilityActorInfo->OwnerActor == AbilitySystemComponent->GetOwnerActor());
		if (AbilitySystemComponent->GetOwnerActor() == nullptr)
		{
			UnInitializeAbilitySystem();
		}
		else
		{
			AbilitySystemComponent->RefreshAbilityActorInfo();
		}
	}

	CheckDefaultInitialization();
}

void UNoodlingPawnExtensionComponent::HandlePlayerStateReplicated()
{
	CheckDefaultInitialization();
}

void UNoodlingPawnExtensionComponent::SetupPlayerInputComponent()
{
	CheckDefaultInitialization();
}

// Called when the game starts
void UNoodlingPawnExtensionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for changes to all features
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);
	
	// Notifies state manager that we have spawned, then try rest of default initialization
	ensure(TryToChangeInitState(NoodlingGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}
