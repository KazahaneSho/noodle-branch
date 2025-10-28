// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/NoodlingCharacterBase.h"

#include "AbilitySystem/NoodlingAbilitySystemComponent.h"
#include "Camera/NoodlingCameraComponent.h"
#include "Character/NoodlingCharacterMovementComponent.h"
#include "Character/NoodlingHealthComponent.h"
#include "Character/NoodlingPawnExtensionComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "NoodleBranch/NoodlingGameplayTags.h"
#include "NoodleBranch/NoodlingLogChannels.h"
#include "Player/NoodlingPlayerController.h"
#include "Player/NoodlingPlayerState.h"
#include "System/NoodlingSignificanceManager.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NoodlingCharacterBase)

static FName NAME_NoodlingCharacterCollisionProfile_Capsule(TEXT("NoodlingPawnCapsule"));
static FName NAME_NoodlingCharacterCollisionProfile_Mesh(TEXT("NoodlingPawnMesh"));

ANoodlingCharacterBase::ANoodlingCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UNoodlingCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Avoid ticking characters if possible, use tick in components instead.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SetNetCullDistanceSquared(900000000.0f);

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);
	CapsuleComp->SetCollisionProfileName(NAME_NoodlingCharacterCollisionProfile_Capsule);

	USkeletalMeshComponent* MeshComp = GetMesh();
	check(MeshComp);
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));  // Rotate mesh to be X forward since it is exported as Y forward.
	MeshComp->SetCollisionProfileName(NAME_NoodlingCharacterCollisionProfile_Mesh);

	UNoodlingCharacterMovementComponent* NoodlingMoveComp = CastChecked<UNoodlingCharacterMovementComponent>(GetCharacterMovement());
	NoodlingMoveComp->GravityScale = 1.0f;
	NoodlingMoveComp->MaxAcceleration = 2400.0f;
	NoodlingMoveComp->BrakingFrictionFactor = 1.0f;
	NoodlingMoveComp->BrakingFriction = 6.0f;
	NoodlingMoveComp->GroundFriction = 8.0f;
	NoodlingMoveComp->BrakingDecelerationWalking = 1400.0f;
	NoodlingMoveComp->bUseControllerDesiredRotation = false;
	NoodlingMoveComp->bOrientRotationToMovement = false;
	NoodlingMoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	NoodlingMoveComp->bAllowPhysicsRotationDuringAnimRootMotion = false;
	NoodlingMoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;
	NoodlingMoveComp->bCanWalkOffLedgesWhenCrouching = true;
	NoodlingMoveComp->SetCrouchedHalfHeight(65.0f);

	PawnExtensionComponentComponent = CreateDefaultSubobject<UNoodlingPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
	PawnExtensionComponentComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	PawnExtensionComponentComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));

	HealthComponent = CreateDefaultSubobject<UNoodlingHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
	HealthComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);

	CameraComponent = CreateDefaultSubobject<UNoodlingCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetRelativeLocation(FVector(-300.0f, 0.0f, 75.0f));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	BaseEyeHeight = 80.0f;
	CrouchedEyeHeight = 50.0f;
}

void ANoodlingCharacterBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void ANoodlingCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	const UWorld* World = GetWorld();
	
	if (/*Register with Significance manager?*/ !IsNetMode(NM_DedicatedServer))
	{
		if (UNoodlingSignificanceManager* SignificanceManager = USignificanceManager::Get<UNoodlingSignificanceManager>(World))
		{
			//This is a Stub
			SignificanceManager->RegisterObject(this, FName("Dummy"), nullptr);
		}
	}
}

void ANoodlingCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	const UWorld* World = GetWorld();
	
	if (/*Registered with Significance manager?*/ !IsNetMode(NM_DedicatedServer))
	{
		if (UNoodlingSignificanceManager* SignificanceManager = USignificanceManager::Get<UNoodlingSignificanceManager>(World))
		{
			SignificanceManager->UnregisterObject(this);
		}
	}
}

void ANoodlingCharacterBase::Reset()
{
	DisableMovementAndCollision();

	K2_OnReset();

	UnInitAndDestroy();
}

void ANoodlingCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, ReplicatedAcceleration, COND_SimulatedOnly);
	DOREPLIFETIME(ThisClass, MyTeamId);
}

void ANoodlingCharacterBase::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	if (const UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		// Compress Acceleration: XY components as direction + magnitude, Z component as direct value
		const double MaxAccel = MovementComponent->MaxAcceleration;
		const FVector CurrentAccel = MovementComponent->GetCurrentAcceleration();
		double AccelXYRadians, AccelXYMagnitude;
		FMath::CartesianToPolar(CurrentAccel.X, CurrentAccel.Y, AccelXYMagnitude, AccelXYRadians);

		ReplicatedAcceleration.AccelXYRadians   = FMath::FloorToInt((AccelXYRadians / TWO_PI) * 255.0);     // [0, 2PI] -> [0, 255]
		ReplicatedAcceleration.AccelXYMagnitude = FMath::FloorToInt((AccelXYMagnitude / MaxAccel) * 255.0);	// [0, MaxAccel] -> [0, 255]
		ReplicatedAcceleration.AccelZ           = FMath::FloorToInt((CurrentAccel.Z / MaxAccel) * 127.0);   // [-MaxAccel, MaxAccel] -> [-127, 127]
	}
}

void ANoodlingCharacterBase::NotifyControllerChanged()
{
	const FGenericTeamId OldTeamId = GetGenericTeamId();

	Super::NotifyControllerChanged();

	// Update our team Id based on the controller
	if (HasAuthority() && (GetController() != nullptr))
	{
		if (const INoodlingTeamAgentInterface* ControllerWithTeam = Cast<INoodlingTeamAgentInterface>(GetController()))
		{
			MyTeamId = ControllerWithTeam->GetGenericTeamId();
			ConditionalBroadcastTeamChanged(this, OldTeamId, MyTeamId);
		}
	}
}

void ANoodlingCharacterBase::SetGenericTeamId(const FGenericTeamId& NewTeamId)
{
	if (GetController() == nullptr)
	{
		if (HasAuthority())
		{
			const FGenericTeamId OldTeamId = MyTeamId;
			MyTeamId = NewTeamId;
			ConditionalBroadcastTeamChanged(this, OldTeamId, MyTeamId);
		}
		else
		{
			UE_LOG(NoodleBranch_LogTeams, Error, TEXT("You can't set the team Id on a character (%s) without HasAuthority()"), *GetPathNameSafe(this));
		}
	}
	else
	{
		UE_LOG(NoodleBranch_LogTeams, Error, TEXT("You can't set the team Id on a possessed character (%s); it's driven by the associated controller"), *GetPathNameSafe(this));
	}
}

FGenericTeamId ANoodlingCharacterBase::GetGenericTeamId() const
{
	return MyTeamId;
}

FOnNoodlingTeamIndexChangedDelegate* ANoodlingCharacterBase::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

ANoodlingPlayerController* ANoodlingCharacterBase::GetNoodlingPlayerController() const
{
	return CastChecked<ANoodlingPlayerController>(GetController(), ECastCheckedType::NullAllowed);
}

ANoodlingPlayerState* ANoodlingCharacterBase::GetNoodlingPlayerState() const
{
	return CastChecked<ANoodlingPlayerState>(GetPlayerState(), ECastCheckedType::NullAllowed);
}

void ANoodlingCharacterBase::OnMovementModeChanged(const EMovementMode PrevMovementMode, const uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	const UNoodlingCharacterMovementComponent* NoodlingMoveComp = CastChecked<UNoodlingCharacterMovementComponent>(GetCharacterMovement());

	SetMovementModeTag(PrevMovementMode, PreviousCustomMode, false);
	SetMovementModeTag(NoodlingMoveComp->MovementMode, NoodlingMoveComp->CustomMovementMode, true);
}

UNoodlingAbilitySystemComponent* ANoodlingCharacterBase::GetNoodlingAbilitySystemComponent() const
{
	return Cast<UNoodlingAbilitySystemComponent>(GetAbilitySystemComponent());
}

UAbilitySystemComponent* ANoodlingCharacterBase::GetAbilitySystemComponent() const
{
	if (PawnExtensionComponentComponent == nullptr)
	{
		return nullptr;
	}

	return PawnExtensionComponentComponent->GetNoodlingAbilitySystemComponent();
}

void ANoodlingCharacterBase::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (const UNoodlingAbilitySystemComponent* NoodlingAbilitySystemComponent = GetNoodlingAbilitySystemComponent())
	{
		NoodlingAbilitySystemComponent->GetOwnedGameplayTags(TagContainer);
	}
}

bool ANoodlingCharacterBase::HasMatchingGameplayTag(const FGameplayTag TagToCheck) const
{
	if (const UNoodlingAbilitySystemComponent* NoodlingAbilitySystemComponent = GetNoodlingAbilitySystemComponent())
	{
		return NoodlingAbilitySystemComponent->HasMatchingGameplayTag(TagToCheck);
	}

	return false;
}

bool ANoodlingCharacterBase::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const UNoodlingAbilitySystemComponent* NoodlingAbilitySystemComponent = GetNoodlingAbilitySystemComponent())
	{
		return NoodlingAbilitySystemComponent->HasAllMatchingGameplayTags(TagContainer);
	}

	return false;
}

bool ANoodlingCharacterBase::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const UNoodlingAbilitySystemComponent* NoodlingAbilitySystemComponent = GetNoodlingAbilitySystemComponent())
	{
		return NoodlingAbilitySystemComponent->HasAnyMatchingGameplayTags(TagContainer);
	}

	return false;
}

void ANoodlingCharacterBase::ToggleCrouch()
{
	const UNoodlingCharacterMovementComponent* NoodlingCharacterMovementComponent = CastChecked<UNoodlingCharacterMovementComponent>(GetCharacterMovement());

	if (IsCrouched() || NoodlingCharacterMovementComponent->bWantsToCrouch)
	{
		UnCrouch();
	}
	else if (NoodlingCharacterMovementComponent->IsMovingOnGround())
	{
		Crouch();
	}
}

void ANoodlingCharacterBase::FastSharedReplication_Implementation(const FSharedRepMovement& SharedRepMovement)
{
	if (GetWorld()->IsPlayingReplay())
	{
		return;
	}

	// Timestamp is checked to reject old moves.
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		// Timestamp
		SetReplicatedServerLastTransformUpdateTimeStamp(SharedRepMovement.RepTimeStamp);

		// Movement mode
		if (GetReplicatedMovementMode() != SharedRepMovement.RepMovementMode)
		{
			SetReplicatedMovementMode(SharedRepMovement.RepMovementMode);
			GetCharacterMovement()->bNetworkMovementModeChanged = true;
			GetCharacterMovement()->bNetworkUpdateReceived = true;
		}

		// Location, Rotation, Velocity, etc.
		FRepMovement& MutableRepMovement = GetReplicatedMovement_Mutable();
		MutableRepMovement = SharedRepMovement.RepMovement;

		// This also sets LastRepMovement
		OnRep_ReplicatedMovement();

		// Jump Force
		SetProxyIsJumpForceApplied(SharedRepMovement.bProxyIsJumpForceApplied);

		// Crouch
		if (IsCrouched() != SharedRepMovement.bIsCrouched)
		{
			SetIsCrouched(SharedRepMovement.bIsCrouched);
			OnRep_IsCrouched();
		}
	}
}

bool ANoodlingCharacterBase::UpdateSharedReplication()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (FSharedRepMovement SharedMovement; SharedMovement.FillForCharacter(this))
		{
			// Only call FastSharedReplication if data has changed since the last frame.
			// Skipping this call will cause replication to reuse the same bunch that we previously
			// produced, but not send it to clients that already received. (But a new client who has not received
			// it, will get it this frame)
			if (!SharedMovement.Equals(LastSharedReplication, this))
			{
				LastSharedReplication = SharedMovement;
				SetReplicatedMovementMode(SharedMovement.RepMovementMode);

				FastSharedReplication(SharedMovement);
			}
			return true;
		}
	}

	// We cannot use FastSharedReplication right now. Don't send anything.
	return false;
}



void ANoodlingCharacterBase::OnAbilitySystemInitialized()
{
	UNoodlingAbilitySystemComponent* NoodlingAbilitySystemComponent = GetNoodlingAbilitySystemComponent();
	check(NoodlingAbilitySystemComponent);

	HealthComponent->InitializeWithAbilitySystem(NoodlingAbilitySystemComponent);

	InitializeGameplayTags();	
}

void ANoodlingCharacterBase::OnAbilitySystemUninitialized()
{
	HealthComponent->UnInitializeFromAbilitySystem();
}

void ANoodlingCharacterBase::PossessedBy(AController* NewController)
{
	const FGenericTeamId OldTeamId = MyTeamId;

	Super::PossessedBy(NewController);

	PawnExtensionComponentComponent->HandleControllerChanged();

	// Grab the current team Id and listen for future changes
	if (INoodlingTeamAgentInterface* ControllerAsTeamProvider = Cast<INoodlingTeamAgentInterface>(NewController))
	{
		MyTeamId = ControllerAsTeamProvider->GetGenericTeamId();
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
	}
	ConditionalBroadcastTeamChanged(this, OldTeamId, MyTeamId);
}

void ANoodlingCharacterBase::UnPossessed()
{
	AController* const OldController = GetController();

	// Stop listening for changes from the old controller
	const FGenericTeamId OldTeamId = MyTeamId;
	if (INoodlingTeamAgentInterface* ControllerAsTeamProvider = Cast<INoodlingTeamAgentInterface>(OldController))
	{
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
	}

	Super::UnPossessed();

	PawnExtensionComponentComponent->HandleControllerChanged();

	// Determine what the new team Id should be afterward.
	MyTeamId = DetermineNewTeamAfterPossessionEnds(OldTeamId);
	ConditionalBroadcastTeamChanged(this, OldTeamId, MyTeamId);
}

void ANoodlingCharacterBase::OnRep_Controller()
{
	Super::OnRep_Controller();

	PawnExtensionComponentComponent->HandleControllerChanged();
}

void ANoodlingCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	PawnExtensionComponentComponent->HandlePlayerStateReplicated();
}

void ANoodlingCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PawnExtensionComponentComponent->SetupPlayerInputComponent();
}

void ANoodlingCharacterBase::InitializeGameplayTags() const
{
	// Clear tags that may be lingering on the ability system from the previous pawn.
	if (UNoodlingAbilitySystemComponent* NoodlingAbilitySystemComponent = GetNoodlingAbilitySystemComponent())
	{
		for (const TPair<uint8, FGameplayTag>& TagMapping : NoodlingGameplayTags::MovementModeTagMap)
		{
			if (TagMapping.Value.IsValid())
			{
				NoodlingAbilitySystemComponent->SetLooseGameplayTagCount(TagMapping.Value, 0);
			}
		}

		for (const TPair<uint8, FGameplayTag>& TagMapping : NoodlingGameplayTags::CustomMovementModeTagMap)
		{
			if (TagMapping.Value.IsValid())
			{
				NoodlingAbilitySystemComponent->SetLooseGameplayTagCount(TagMapping.Value, 0);
			}
		}

		const UNoodlingCharacterMovementComponent* NoodlingMoveComponent = CastChecked<UNoodlingCharacterMovementComponent>(GetCharacterMovement());
		SetMovementModeTag(NoodlingMoveComponent->MovementMode, NoodlingMoveComponent->CustomMovementMode, true);
	}
}

void ANoodlingCharacterBase::FellOutOfWorld(const UDamageType& dmgType)
{
	HealthComponent->DamageSelfDestruct(/*bFellOutOfWorld=*/ true);
}

void ANoodlingCharacterBase::OnDeathStarted(AActor* OwningActor)
{
	DisableMovementAndCollision();
}

void ANoodlingCharacterBase::OnDeathFinished(AActor* OwningActor)
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::DestroyDueToDeath);
}

void ANoodlingCharacterBase::DisableMovementAndCollision() const
{
	if (GetController())
	{
		GetController()->SetIgnoreMoveInput(true);
	}

	UCapsuleComponent* ThisCapsuleComponent = GetCapsuleComponent();
	check(ThisCapsuleComponent);
	ThisCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ThisCapsuleComponent->SetCollisionResponseToAllChannels(ECR_Ignore);

	UNoodlingCharacterMovementComponent* NoodlingMovementComponent = CastChecked<UNoodlingCharacterMovementComponent>(GetCharacterMovement());
	NoodlingMovementComponent->StopMovementImmediately();
	NoodlingMovementComponent->DisableMovement();
}

void ANoodlingCharacterBase::DestroyDueToDeath()
{
	K2_OnDeathFinished();

	UnInitAndDestroy();
}

void ANoodlingCharacterBase::UnInitAndDestroy()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		DetachFromControllerPendingDestroy();
		SetLifeSpan(0.1f);
	}

	// Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
	if (const UNoodlingAbilitySystemComponent* NoodlingAbilitySystemComponent = GetNoodlingAbilitySystemComponent())
	{
		if (NoodlingAbilitySystemComponent->GetAvatarActor() == this)
		{
			PawnExtensionComponentComponent->UnInitializeAbilitySystem();
		}
	}

	SetActorHiddenInGame(true);
}

void ANoodlingCharacterBase::SetMovementModeTag(const EMovementMode MovementMode, const uint8 CustomMovementMode, const bool bTagEnabled) const
{
	if (UNoodlingAbilitySystemComponent* NoodlingAbilitySystemComponent = GetNoodlingAbilitySystemComponent())
	{
		const FGameplayTag* MovementModeTag;
		if (MovementMode == MOVE_Custom)
		{
			MovementModeTag = NoodlingGameplayTags::CustomMovementModeTagMap.Find(CustomMovementMode);
		}
		else
		{
			MovementModeTag = NoodlingGameplayTags::MovementModeTagMap.Find(MovementMode);
		}

		if (MovementModeTag && MovementModeTag->IsValid())
		{
			NoodlingAbilitySystemComponent->SetLooseGameplayTagCount(*MovementModeTag, (bTagEnabled ? 1 : 0));
		}
	}
}

void ANoodlingCharacterBase::OnStartCrouch(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	if (UNoodlingAbilitySystemComponent* NoodlingAbilitySystemComponent = GetNoodlingAbilitySystemComponent())
	{
		NoodlingAbilitySystemComponent->SetLooseGameplayTagCount(NoodlingGameplayTags::Status_Crouching, 1);
	}


	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void ANoodlingCharacterBase::OnEndCrouch(const float HalfHeightAdjust, const float ScaledHalfHeightAdjust)
{
	if (UNoodlingAbilitySystemComponent* NoodlingAbilitySystemComponent = GetNoodlingAbilitySystemComponent())
	{
		NoodlingAbilitySystemComponent->SetLooseGameplayTagCount(NoodlingGameplayTags::Status_Crouching, 0);
	}

	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

bool ANoodlingCharacterBase::CanJumpInternal_Implementation() const
{
	// same as ACharacter's implementation but without the crouch check
	return JumpIsAllowedInternal();
}

void ANoodlingCharacterBase::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, const int32 NewTeam)
{
	const FGenericTeamId MyOldTeamId = MyTeamId;
	MyTeamId = IntegerToGenericTeamId(NewTeam);
	ConditionalBroadcastTeamChanged(this, MyOldTeamId, MyTeamId);
}

void ANoodlingCharacterBase::OnRep_ReplicatedAcceleration() const
{if (UNoodlingCharacterMovementComponent* NoodlingMovementComponent = Cast<UNoodlingCharacterMovementComponent>(GetCharacterMovement()))
{
	// Decompress Acceleration
	const double MaxAccel         = NoodlingMovementComponent->MaxAcceleration;
	const double AccelXYMagnitude = static_cast<double>(ReplicatedAcceleration.AccelXYMagnitude) * MaxAccel / 255.0; // [0, 255] -> [0, MaxAccel]
	const double AccelXYRadians   = static_cast<double>(ReplicatedAcceleration.AccelXYRadians) * TWO_PI / 255.0;     // [0, 255] -> [0, 2PI]

	FVector UnpackedAcceleration(FVector::ZeroVector);
	FMath::PolarToCartesian(AccelXYMagnitude, AccelXYRadians, UnpackedAcceleration.X, UnpackedAcceleration.Y);
	UnpackedAcceleration.Z = static_cast<double>(ReplicatedAcceleration.AccelZ) * MaxAccel / 127.0; // [-127, 127] -> [-MaxAccel, MaxAccel]

	NoodlingMovementComponent->SetReplicatedAcceleration(UnpackedAcceleration);
}
}

void ANoodlingCharacterBase::OnRep_MyTeamId(const FGenericTeamId OldTeamId)
{
	ConditionalBroadcastTeamChanged(this, OldTeamId, MyTeamId);
}

FSharedRepMovement::FSharedRepMovement()
{
	RepMovement.LocationQuantizationLevel = EVectorQuantization::RoundTwoDecimals;
}

bool FSharedRepMovement::FillForCharacter(const ACharacter* Character)
{
	if (const USceneComponent* PawnRootComponent = Character->GetRootComponent())
	{
		const UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement();

		RepMovement.Location = FRepMovement::RebaseOntoZeroOrigin(PawnRootComponent->GetComponentLocation(), Character);
		RepMovement.Rotation = PawnRootComponent->GetComponentRotation();
		RepMovement.LinearVelocity = CharacterMovement->Velocity;
		RepMovementMode = CharacterMovement->PackNetworkMovementMode();
		bProxyIsJumpForceApplied = Character->GetProxyIsJumpForceApplied() || (Character->JumpForceTimeRemaining > 0.0f);
		bIsCrouched = Character->IsCrouched();

		// Timestamp is sent as zero if unused
		if ((CharacterMovement->NetworkSmoothingMode == ENetworkSmoothingMode::Linear) || CharacterMovement->bNetworkAlwaysReplicateTransformUpdateTimestamp)
		{
			RepTimeStamp = CharacterMovement->GetServerLastTransformUpdateTimeStamp();
		}
		else
		{
			RepTimeStamp = 0.f;
		}

		return true;
	}
	return false;
}

bool FSharedRepMovement::Equals(const FSharedRepMovement& Other, ACharacter* Character) const
{
	if (RepMovement.Location != Other.RepMovement.Location)
	{
		return false;
	}

	if (RepMovement.Rotation != Other.RepMovement.Rotation)
	{
		return false;
	}

	if (RepMovement.LinearVelocity != Other.RepMovement.LinearVelocity)
	{
		return false;
	}

	if (RepMovementMode != Other.RepMovementMode)
	{
		return false;
	}

	if (bProxyIsJumpForceApplied != Other.bProxyIsJumpForceApplied)
	{
		return false;
	}

	if (bIsCrouched != Other.bIsCrouched)
	{
		return false;
	}

	return true;
}

bool FSharedRepMovement::NetSerialize(FArchive& Archive, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;
	RepMovement.NetSerialize(Archive, Map, bOutSuccess);
	Archive << RepMovementMode;
	Archive << bProxyIsJumpForceApplied;
	Archive << bIsCrouched;

	// Timestamp, if non-zero.
	uint8 bHasTimeStamp = (RepTimeStamp != 0.f);
	Archive.SerializeBits(&bHasTimeStamp, 1);
	if (bHasTimeStamp)
	{
		Archive << RepTimeStamp;
	}
	else
	{
		RepTimeStamp = 0.f;
	}

	return true;
}
