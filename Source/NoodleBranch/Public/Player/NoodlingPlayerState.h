// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularPlayerState.h"
#include "NoodlingPlayerState.generated.h"

#define UE_API NOODLEBRANCH_API

class UNoodlingAbilitySystemComponent;
class UNoodlingPawnData;
/**
 * Project Base PlayerState class.
 */
UCLASS()
class ANoodlingPlayerState : public AModularPlayerState
{
	GENERATED_BODY()

public:
	UE_API explicit ANoodlingPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "NoodleBranch|PlayerState")
	UNoodlingAbilitySystemComponent* GetNoodlingAbilitySystemComponent() const { return AbilitySystemComponent; }
	
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

protected:
	
	UFUNCTION()
	UE_API void OnRep_PawnData();

protected:
	// TODO: Add replication
	UPROPERTY()
	TObjectPtr<const UNoodlingPawnData> PawnData;

private:

	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "NoodleBranch|PlayerState")
	TObjectPtr<UNoodlingAbilitySystemComponent> AbilitySystemComponent;
};

#undef UE_API
