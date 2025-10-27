﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemInterface.h"
#include "GameplayCueInterface.h"
#include "GameplayTagAssetInterface.h"
#include "ModularCharacter.h"
#include "Teams/NoodlingTeamAgentInterface.h"
#include "NoodlingCharacterBase.generated.h"

#define UE_API NOODLEBRANCH_API

class UNoodlingPawnExtensionComponent;
class UNoodlingAbilitySystemComponent;
class ANoodlingPlayerState;
class ANoodlingPlayerController;
class UNoodlingHealthComponent;
class UNoodlingCameraComponent;

/**
 * FNoodlingReplicatedAcceleration: Compressed representation of acceleration
 */
USTRUCT()
struct FNoodlingReplicatedAcceleration
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 AccelXYRadians = 0;	// Direction of XY accel component, quantized to represent [0, 2*pi]

	UPROPERTY()
	uint8 AccelXYMagnitude = 0;	//Accel rate of XY component, quantized to represent [0, MaxAcceleration]

	UPROPERTY()
	int8 AccelZ = 0;	// Raw Z accel rate component, quantized to represent [-MaxAcceleration, MaxAcceleration]
};

/** The type we use to send FastShared movement updates. */
USTRUCT()
struct FSharedRepMovement
{
	GENERATED_BODY()

	FSharedRepMovement();

	bool FillForCharacter(const ACharacter* Character);
	bool Equals(const FSharedRepMovement& Other, ACharacter* Character) const;

	bool NetSerialize(FArchive& Archive, UPackageMap* Map, bool& bOutSuccess);

	UPROPERTY(Transient)
	FRepMovement RepMovement;

	UPROPERTY(Transient)
	float RepTimeStamp = 0.0f;

	UPROPERTY(Transient)
	uint8 RepMovementMode = 0;

	UPROPERTY(Transient)
	bool bProxyIsJumpForceApplied = false;

	UPROPERTY(Transient)
	bool bIsCrouched = false;
};

template<>
struct TStructOpsTypeTraits<FSharedRepMovement> : TStructOpsTypeTraitsBase2<FSharedRepMovement>
{
	enum
	{
		WithNetSerializer = true,
		WithNetSharedSerialization = true,
	};
};

/**
 * ANoodlingCharacterBase
 *
 *	The base character pawn class used by this project.
 *	Responsible for sending events to pawn components.
 *	New behavior should be added via pawn components when possible.
 */
UCLASS(MinimalAPI, Config=Game, meta=(ShortTooltip="The base character pawn class used by this project."))
class ANoodlingCharacterBase : public AModularCharacter, public IAbilitySystemInterface, public IGameplayCueInterface, public IGameplayTagAssetInterface, public INoodlingTeamAgentInterface
{
	GENERATED_BODY()
	
public:

	UE_API explicit ANoodlingCharacterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AActor interface
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UE_API virtual void Reset() override;
	UE_API virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UE_API virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	//~End of AActor interface

	//~APawn interface
	UE_API virtual void NotifyControllerChanged() override;
	//~End of APawn interface

	//~INoodlingTeamAgentInterface interface
	UE_API virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId) override;
	UE_API virtual FGenericTeamId GetGenericTeamId() const override;
	UE_API virtual FOnNoodlingTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of INoodlingTeamAgentInterface interface

	UFUNCTION(BlueprintCallable, Category = "NoodleBranch|Character")
	UE_API ANoodlingPlayerController* GetNoodlingPlayerController() const;

	UFUNCTION(BlueprintCallable, Category = "NoodleBranch|Character")
	UE_API ANoodlingPlayerState* GetNoodlingPlayerState() const;

protected:
	
	UE_API virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

public:

	UFUNCTION(BlueprintCallable, Category = "NoodleBranch|Character")
	UE_API UNoodlingAbilitySystemComponent* GetNoodlingAbilitySystemComponent() const;
	UE_API virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UE_API virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	UE_API virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	UE_API virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	UE_API virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;

	UE_API void ToggleCrouch();

	/** RPCs that is called on frames when default property replication is skipped. This replicates a single movement update to everyone. */
	UFUNCTION(NetMulticast, unreliable)
	UE_API void FastSharedReplication(const FSharedRepMovement& SharedRepMovement);

	// Last FSharedRepMovement we sent, to avoid sending repeatedly.
	FSharedRepMovement LastSharedReplication;

	UE_API virtual bool UpdateSharedReplication();

protected:

	UE_API virtual void OnAbilitySystemInitialized();
	UE_API virtual void OnAbilitySystemUninitialized();

	UE_API virtual void PossessedBy(AController* NewController) override;
	UE_API virtual void UnPossessed() override;

	UE_API virtual void OnRep_Controller() override;
	UE_API virtual void OnRep_PlayerState() override;

	UE_API virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UE_API void InitializeGameplayTags() const;

	UE_API virtual void FellOutOfWorld(const UDamageType& dmgType) override;

	// Begins the death sequence for the character (disables collision, disables movement, etc...)
	UFUNCTION()
	UE_API virtual void OnDeathStarted(AActor* OwningActor);

	// Ends the death sequence for the character (detaches controller, destroys pawn, etc...)
	UFUNCTION()
	UE_API virtual void OnDeathFinished(AActor* OwningActor);

	UE_API void DisableMovementAndCollision() const;
	UE_API void DestroyDueToDeath();
	UE_API void UnInitAndDestroy();

	// Called when the death sequence for the character has completed
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnDeathFinished"))
	UE_API void K2_OnDeathFinished();

	UE_API void SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled) const;

	UE_API virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	UE_API virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	UE_API virtual bool CanJumpInternal_Implementation() const override;

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NoodleBranch|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNoodlingPawnExtensionComponent> PawnExtensionComponentComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NoodleBranch|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNoodlingHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NoodleBranch|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNoodlingCameraComponent> CameraComponent;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_ReplicatedAcceleration)
	FNoodlingReplicatedAcceleration ReplicatedAcceleration;

	UPROPERTY(ReplicatedUsing = OnRep_MyTeamId)
	FGenericTeamId MyTeamId;

	UPROPERTY()
	FOnNoodlingTeamIndexChangedDelegate OnTeamChangedDelegate;

protected:
	// Called to determine what happens to the team ID when possession ends
	virtual FGenericTeamId DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamId) const
	{
		// This could be changed to return, e.g., OldTeamId if you want to keep it assigned afterward, or return an ID for some neutral faction, or etc...
		return FGenericTeamId::NoTeam;
	}

private:
	
	UFUNCTION()
	UE_API void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

	UFUNCTION()
	UE_API void OnRep_ReplicatedAcceleration() const;

	UFUNCTION()
	UE_API void OnRep_MyTeamId(FGenericTeamId OldTeamId);
};

#undef UE_API
