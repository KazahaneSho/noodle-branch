// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemInterface.h"
#include "ModularPlayerState.h"
#include "System/GameplayTagStack.h"
#include "Teams/NoodlingTeamAgentInterface.h"
#include "NoodlingPlayerState.generated.h"

#define UE_API NOODLEBRANCH_API

struct FNoodlingVerbMessage;

class AController;
class ANoodlingPlayerController;
class APlayerState;
class FName;
class UAbilitySystemComponent;
class UNoodlingAbilitySystemComponent;
class UNoodlingExperienceDefinition;
class UNoodlingPawnData;
class UObject;
struct FFrame;
struct FGameplayTag;


class ANoodlingPlayerController;
/** Defines the types of client connected */
UENUM()
enum class ENoodlingPlayerConnectionType : uint8
{
	// An active player
	Player = 0,

	// Spectator connected to a running game
	LiveSpectator,

	// Spectating a demo recording offline
	ReplaySpectator,

	// A deactivated player (disconnected)
	InactivePlayer
};

/**
 * Project Base PlayerState class.
 */
UCLASS()
class ANoodlingPlayerState : public AModularPlayerState, public IAbilitySystemInterface, public INoodlingTeamAgentInterface
{
	GENERATED_BODY()

public:
	UE_API explicit ANoodlingPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AActor interface
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void PostInitializeComponents() override;
	//~End of AActor interface

	//~APlayerState interface
	UE_API virtual void Reset() override;
	UE_API virtual void ClientInitialize(AController* C) override;
	UE_API virtual void CopyProperties(APlayerState* PlayerState) override;
	UE_API virtual void OnDeactivated() override;
	UE_API virtual void OnReactivated() override;
	//~End of APlayerState interface

	//~INoodlingTeamAgentInterface interface
	UE_API virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	UE_API virtual FGenericTeamId GetGenericTeamId() const override;
	UE_API virtual FOnNoodlingTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of INoodlingTeamAgentInterface interface
	
	UE_API virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "Noodling|PlayerState")
	UE_API ANoodlingPlayerController* GetNoodlingPlayerController() const;
	
	UFUNCTION(BlueprintCallable, Category = "NoodleBranch|PlayerState")
	UNoodlingAbilitySystemComponent* GetNoodlingAbilitySystemComponent() const { return AbilitySystemComponent; }
	
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }
	UE_API void SetPawnData(const UNoodlingPawnData* InPawnData);

	static UE_API const FName NAME_NoodlingAbilityReady;

	UE_API void SetPlayerConnectionType(ENoodlingPlayerConnectionType NewType);
	ENoodlingPlayerConnectionType GetPlayerConnectionType() const { return MyPlayerConnectionType; }

	/** Returns the Squad ID of the squad the player belongs to. */
	UFUNCTION(BlueprintCallable)
	int32 GetSquadId() const
	{
		return MySquadID;
	}

	/** Returns the Team ID of the team the player belongs to. */
	UFUNCTION(BlueprintCallable)
	int32 GetTeamId() const
	{
		return GenericTeamIdToInteger(MyTeamID);
	}

	UE_API void SetSquadID(int32 NewSquadId);

	// Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Teams)
	UE_API void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Teams)
	UE_API void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Returns the stack count of the specified tag (or 0 if the tag is not present)
	UFUNCTION(BlueprintCallable, Category=Teams)
	UE_API int32 GetStatTagStackCount(FGameplayTag Tag) const;

	// Returns true if there is at least one stack of the specified tag
	UFUNCTION(BlueprintCallable, Category=Teams)
	UE_API bool HasStatTag(FGameplayTag Tag) const;

	// Send a message to just this player
	// (use only for client notifications like accolades, quest toasts, etc... that can handle being occasionally lost)
	UFUNCTION(Client, Unreliable, BlueprintCallable, Category = "NoodleBranch|PlayerState")
	UE_API void ClientBroadcastMessage(const FNoodlingVerbMessage Message);

	// Gets the replicated view rotation of this player, used for spectating
	UE_API FRotator GetReplicatedViewRotation() const;

	// Sets the replicated view rotation, only valid on the server
	UE_API void SetReplicatedViewRotation(const FRotator& NewRotation);

protected:
	
	UFUNCTION()
	UE_API void OnRep_PawnData();

protected:
	UPROPERTY(ReplicatedUsing = OnRep_PawnData)
	TObjectPtr<const UNoodlingPawnData> PawnData;

private:

	UE_API void OnExperienceLoaded(const UNoodlingExperienceDefinition* CurrentExperience);

	UFUNCTION()
	UE_API void OnRep_MyTeamID(FGenericTeamId OldTeamID);

	UFUNCTION()
	UE_API void OnRep_MySquadID();

private:

	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "NoodleBranch|PlayerState")
	TObjectPtr<UNoodlingAbilitySystemComponent> AbilitySystemComponent;

	// Health attribute set used by this actor.
	UPROPERTY()
	TObjectPtr<const class UNoodlingHealthSet> HealthSet;
	// Combat attribute set used by this actor.
	UPROPERTY()
	TObjectPtr<const class UNoodlingCombatSet> CombatSet;

	UPROPERTY(Replicated)
	ENoodlingPlayerConnectionType MyPlayerConnectionType;

	UPROPERTY()
	FOnNoodlingTeamIndexChangedDelegate OnTeamChangedDelegate;

	UPROPERTY(ReplicatedUsing=OnRep_MyTeamID)
	FGenericTeamId MyTeamID;

	UPROPERTY(ReplicatedUsing=OnRep_MySquadID)
	int32 MySquadID;

	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;

	UPROPERTY(Replicated)
	FRotator ReplicatedViewRotation;
};

#undef UE_API
