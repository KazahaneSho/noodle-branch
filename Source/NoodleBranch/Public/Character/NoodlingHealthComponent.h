// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/GameFrameworkComponent.h"
#include "NoodlingHealthComponent.generated.h"

#define UE_API NOODLEBRANCH_API

class UNoodlingAbilitySystemComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNoodlingHealth_DeathEvent, AActor*, OwningActor);

/**
 * UNoodlingHealthComponent
 *
 *	An actor component used to handle anything related to health.
 */
UCLASS(MinimalAPI, Blueprintable, meta=(BlueprintSpawnableComponent))
class UNoodlingHealthComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UE_API explicit UNoodlingHealthComponent(const FObjectInitializer& ObjectInitializer);

	// Initialize the component using an ability system component.
	UFUNCTION(BlueprintCallable, Category = "NoodleBranch|Health")
	UE_API void InitializeWithAbilitySystem(UNoodlingAbilitySystemComponent* InAbilitySystemComponent);

	// Uninitialize the component, clearing any references to the ability system.
	UFUNCTION(BlueprintCallable, Category = "NoodleBranch|Health")
	UE_API void UnInitializeFromAbilitySystem();

	// Applies enough damage to kill the owner.
	UE_API virtual void DamageSelfDestruct(bool bFellOutOfWorld = false);

public:

	// Delegate fired when the death sequence has started.
	UPROPERTY(BlueprintAssignable)
	FNoodlingHealth_DeathEvent OnDeathStarted;

	// Delegate fired when the death sequence has finished.
	UPROPERTY(BlueprintAssignable)
	FNoodlingHealth_DeathEvent OnDeathFinished;

};

#undef UE_API
