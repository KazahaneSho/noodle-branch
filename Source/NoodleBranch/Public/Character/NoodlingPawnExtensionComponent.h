// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "NoodlingPawnExtensionComponent.generated.h"

#define UE_API NOODLEBRANCH_API


/**
 * Component that adds functionality to all Pawn classes so it can be used for characters/vehicles/etc.
 * This coordinates the initialization of other components.
 */

class UNoodlingAbilitySystemComponent;
class UNoodlingPawnData;

UCLASS(MinimalAPI)
class UNoodlingPawnExtensionComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UE_API explicit UNoodlingPawnExtensionComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "Noodling|Pawn")
	static UNoodlingPawnExtensionComponent* FindPawnExtensionComponent(const AActor* Actor) { return Actor ? Actor->FindComponentByClass<UNoodlingPawnExtensionComponent>() : nullptr; };
	
	// Set current pawn data
	UE_API void SetPawnData(const UNoodlingPawnData* InPawnData);

	/** Register with the OnAbilitySystemInitialized delegate and broadcast if our pawn has been registered with the ability system component */
	UE_API void OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate);

	/** Register with the OnAbilitySystemUninitialized delegate fired when our pawn is removed as the ability system's avatar actor */
	UE_API void OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate);

	/** Gets the current ability system component, which may be owned by a different actor */
	UFUNCTION(BlueprintPure, Category = "Noodling|Pawn")
	UNoodlingAbilitySystemComponent* GetNoodlingAbilitySystemComponent() const { return AbilitySystemComponent; }

	/** Should be called by the owning pawn to become the avatar of the ability system. */
	UE_API void InitializeAbilitySystem(UNoodlingAbilitySystemComponent* InAbilitySystemComponent, AActor* InOwnerActor);

	/** Should be called by the owning pawn to remove itself as the avatar of the ability system. */
	UE_API void UnInitializeAbilitySystem();

	/** Should be called by the owning pawn when the pawn's controller changes. */
	UE_API void HandleControllerChanged();

	/** Should be called by the owning pawn when the player state has been replicated. */
	UE_API void HandlePlayerStateReplicated();

	/** Should be called by the owning pawn when the input component is setup. */
	UE_API void SetupPlayerInputComponent();

protected:
	virtual void BeginPlay() override;

	/** Pointer to the ability system component that is cached for convenience. */
	UPROPERTY(Transient)
	TObjectPtr<UNoodlingAbilitySystemComponent> AbilitySystemComponent;
};

#undef UE_API
